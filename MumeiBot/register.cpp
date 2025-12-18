#include "register.h"

#include <string>
#include <mutex>
#include <format>
#include <regex>

#include <jwt-cpp/jwt.h>
#include <uuid_v4.h>
#include <pqxx/pqxx>
//#include <lexbor/html/parser.h>

#include "util.h"
#include "htmlform.h"
#include "httpheaders.h"

static UUIDv4::UUIDGenerator<std::mt19937_64> uuid_generator;

static std::string authenticate(const HTTPCookies & cookies) {
	if (cookies.has("JWT")) {

		jwt::decoded_jwt jwt = jwt::decode(cookies["JWT"]);
		jwt::claim user = jwt.get_payload_claim("User");
		jwt::verifier verifier = jwt::verify()
			.with_type("JWT")
			.with_issuer("StreamBot")
			.with_claim("User", user)
			.allow_algorithm(jwt::algorithm::hs256("streambot"));
		std::error_code error;
		verifier.verify(jwt, error);

		if (error) return "";

		pqxx::connection pqconn(CONN_STR);
		pqxx::read_transaction work(pqconn);
		pqxx::result table = work.exec_params("SELECT access_token FROM session WHERE id=$1", user.as_string());
		work.commit();
		if (table.size() == 1) {
			return table[0][0].as<std::string>();
		}

		return "";
	}
	return "";
}

static std::string remove_session(const HTTPCookies & cookies) {
	if (cookies.has("JWT")) {

		jwt::decoded_jwt jwt = jwt::decode(cookies["JWT"]);
		jwt::claim user = jwt.get_payload_claim("User");
		jwt::verifier verifier = jwt::verify()
			.with_type("JWT")
			.with_issuer("StreamBot")
			.with_claim("User", user)
			.allow_algorithm(jwt::algorithm::hs256("streambot"));
		std::error_code error;
		verifier.verify(jwt, error);

		if (error) return "";

		pqxx::connection pqconn(CONN_STR);
		pqxx::work work(pqconn);
		pqxx::result table = work.exec_params("SELECT access_token FROM session WHERE id=$1", user.as_string());
		work.exec_params("DELETE FROM session WHERE id=$1", user.as_string());
		work.commit();
		if (table.size() == 1) {
			return table[0][0].as<std::string>();
		}

		return "";
	}
	return "";
}

static std::string normalize_yt_link(const std::string & original) {
	static const std::regex at_channel_regex(R"(https://(?:www\.)youtube\.com/@\w*)");
	static const std::regex id_channel_regex(R"(https://(?:www\.)youtube\.com/channel/)" YT_CHANNEL_ID);
	static const std::regex video_regex(R"(https://(?:www\.)youtube\.com/watch?v=)" YT_VIDEO_ID);
	if (std::regex_match(original, at_channel_regex) || std::regex_match(original, id_channel_regex) ) {
		GET(original, NULL, NULL, NULL);
		static const std::regex RSS_regex(R"-(<link rel="alternate" type="application/rss\+xml" title="RSS" href="(https://(?:www\.)youtube\.com)(/feeds/videos\.xml\?channel_id=)-" YT_CHANNEL_ID R"-()">)-");
		std::smatch match;
		if (std::regex_search(input, match, RSS_regex)) {
			return match[1].str() + "/xml" + match[2].str();
		}
		return "";
	}
	else if (std::regex_match(original, video_regex)) {
		GET(original, NULL, NULL, NULL);
		static const std::regex RSS_regex(R"-(<link rel="alternate" type="application/json+oembed" href="(https://www.youtube.com/oembed?format=json&amp;url=https%3A%2F%2Fwww.youtube.com%2Fwatch%3Fv%3D)-" YT_VIDEO_ID R"-()" title=".*">)-");
		std::smatch match;
		if (std::regex_search(input, match, RSS_regex)) {
			GET(match[1], NULL, NULL, NULL);
			picojson::value embed;
			picojson::parse(embed, input);
			if (embed.get("author_url").is<std::string>()) {
				return normalize_yt_link(embed.get("author_url").to_str());
			}
		}
		return "";
	}
	return "";
}

int remove_bot_callback(struct mg_connection * conn, void * cbdata) {

	HTMLForm data_form = get_body(conn);

	if (data_form.has("guild_id")) {

		curl_slist * header = curl_slist_append(NULL, ("Authorization: Bot " + BOT_TOKEN).c_str());
		DELETE(DISCORD_API "/users/@me/guilds/" + data_form["guild_id"], header, NULL, NULL);
		curl_slist_free_all(header);

		HTTPHeaders response_headers;
		response_headers["Location"] = "/register/";
		response_headers["Connection"] = "close";
		mg_printf(conn,
			"HTTP/1.1 303 See Other\r\n%s", std::string(response_headers).c_str());
		return 1;
	}
	mg_printf(conn, "HTTP/1.1 422 Unprocessable Content\r\n");
	return 1;
}

int add_bot_callback(struct mg_connection * conn, void * cbdata) {

	std::string data;

	int count;
	do {
		char buffer[256];
		count = mg_read(conn, buffer, 255);
		if (count > 0) {
			buffer[count] = 0;
			data += buffer;
		}
	} while(count > 0);

	HTMLForm data_form = data;

	if (data_form.has("guild_id")) {

		HTMLForm auth_form;
		auth_form["client_id"] = CLIENT_ID;
		auth_form["scope"] = "bot";
		auth_form["permissions"] = "19456";
		auth_form["response_type"] = "code";
		auth_form["redirect_uri"] = REDIRECT_URL;
		auth_form["integration_type"] = "0";
		auth_form["guild_id"] = data_form["guild_id"];
		auth_form["disable_guild_select"] = "true";

		std::string url = DISCORD_AUTH "?";
		url += auth_form;

		HTTPHeaders response_headers;
		response_headers["Location"] = url;
		response_headers["Connection"] = "close";

		mg_printf(conn, "HTTP/1.1 303 See Other\r\n%s", std::string(response_headers).c_str());
		return 1;
	}
	mg_printf(conn, "HTTP/1.1 422 Unprocessable Content\r\n");
	return 1;
}

void revoke_token(const std::string & access_token) {
		HTMLForm form;
		form["token"] = access_token;
		form["token_type_hint"] = "access_token";
		form["client_id"] = CLIENT_ID;
		form["client_secret"] = CLIENT_SECRET;
		output = form;
		output_type = "application/x-www-form-urlencoded";
		POST(TOKEN_URL "/revoke", NULL, NULL, NULL);
}

int login_callback(struct mg_connection * conn, void * cbdata) {
	HTTPHeaders response_headers;
	response_headers["Content-Type"] = "text/html; charset=utf-8";
	response_headers["Connection"] = "close";
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n%s", std::string(response_headers).c_str());
	mg_printf(conn, load_file(WEB_ROOT "register/login/index.html").c_str(), AUTH_URL);
	return 1;
}

int logout_callback(struct mg_connection * conn, void * cbdata) {

	const struct mg_request_info * info = mg_get_request_info(conn);
	HTTPHeaders request_headers(info->http_headers, info->num_headers);

	std::string auth_token = remove_session(request_headers.cookies);
	if (!auth_token.empty()) {
		revoke_token(auth_token);
	}

	HTTPHeaders response_headers;
	response_headers["Location"] = "/register/login/";
	response_headers["Clear-Site-Data"] = "\"cookies\"";
	response_headers["Connection"] = "close";
	mg_printf(conn, "HTTP/1.1 303 See Other\r\n%s", std::string(response_headers).c_str());

	return 1;
}

static std::string get_guild_form(const std::string & auth_token) {
	std::stringstream html;

	std::vector<Guild> guilds = get_guilds(auth_token);

	html << "<form>";
	html << "<lable for='guild'>Guild: </lable>";
	html << "<select name='guild' id='guild'>";
	for (const Guild & guild : guilds) {
		if (guild.permissions & ((1 << 5)))
			html << "<option value='" << guild.id << "'>" << guild.name << "</option>";
	}
	html << "</select>";
	html << "<input type='submit' value='Next'>";
	html << "</form>";

	return html.str();
}

static std::string get_channels_form(int64_t guild_id, const std::string & bot_token) {
	std::stringstream html;

	std::vector<Channel> channels = get_guild_channels(guild_id, bot_token);

	html << "<form>";
	html << "<lable for='channel'>Channel: </lable>";
	html << "<select name='channel' id='channel'>";
	for (const Channel & channel : channels) {
		if (channel.type == Channel::GUILD_TEXT || channel.type == Channel::GUILD_ANNOUNCEMENT)
			html << "<option value='" << channel.id << "'>" << channel.name << "</option>";
	}
	html << "</select>";
	html << "<lable for='yt'>Youtube Channel Link: </lable>";
	html << "<input id='yt' name='yt' type='text'>";
	html << "<input type='submit' value='Subscribe'>";
	html << "</form>";

	return html.str();
}

int register_guild_add_entry_callback(struct mg_connection * conn, void * cbdata) {

	const struct mg_request_info * info = mg_get_request_info(conn);

	std::string path = info->local_uri;
	std::string guild = path.substr(std::strlen("/register/"));
	guild = guild.substr(0, guild.find('/'));

	if (std::all_of(guild.begin(), guild.end(), [](const unsigned char & c) { return isdigit(c); })) {

		HTTPHeaders request_headers(info->http_headers, info->num_headers);

		std::string auth_token = authenticate(request_headers.cookies);

		if (!auth_token.empty()) {

			int64_t guild_id = std::stoll(guild);

			HTMLForm query = get_body(conn); // TODO: rename text_channel field

			std::string yt_channel = normalize_yt_link(query["yt_channel"]);

			if (!yt_channel.empty()) {
				pqxx::connection pgconn(CONN_STR);
				pqxx::work work(pgconn);
				work.exec("CREATE TABLE IF NOT EXISTS subscription(id SERIAL PRIMARY KEY, guild BIGINT, text_channel BIGINT, yt_channel VARCHAR(252))");
				work.exec_params("INSERT INTO subscription(guild, text_channel, yt_channel) VALUES ($1, $2, $3)", guild, query["guild_channel_id"], query["yt_channel"]);
				
				HTMLForm rss_form;
				rss_form["text_channel"] = query["guild_channel_id"];
				long long lease;
				subscribe_RSS(yt_channel, rss_form, &lease);

				if (!lease) {
					work.abort(); // TODO: error page
				}
				else {
					work.commit();
				}
			}
			mg_send_http_redirect(conn, ("/register/" + guild).c_str(), 303);
			return 1;
		}

		mg_send_http_redirect(conn, "/register/login/", 303);
		return 1;
	}
	return 0;
}

int register_guild_remove_entry_callback(struct mg_connection * conn, void * cbdata) {

	const struct mg_request_info * info = mg_get_request_info(conn);

	std::string path = info->local_uri;
	std::string guild = path.substr(std::strlen("/register/"));
	guild = guild.substr(0, guild.find('/'));

	if (std::all_of(guild.begin(), guild.end(), [](const unsigned char & c) {return isdigit(c);})) {

		HTTPHeaders request_headers(info->http_headers, info->num_headers);

		std::string auth_token = authenticate(request_headers.cookies);

		if (!auth_token.empty()) {

			int64_t guild_id = std::stoll(guild);

			HTMLForm query = get_body(conn);

			pqxx::connection pgconn(CONN_STR);
			pqxx::work work(pgconn);
			work.exec("CREATE TABLE IF NOT EXISTS subscription(id SERIAL PRIMARY KEY, guild BIGINT, text_channel BIGINT, yt_channel VARCHAR(252))");
			work.exec_params("DELETE FROM subscription WHERE guild=$1 AND text_channel=$2", guild, query["channel"]);
			work.commit();
			mg_send_http_redirect(conn, ("/register/" + guild).c_str(), 303);
			return 1;
		}

		mg_send_http_redirect(conn, "/register/login/", 303);
		return 1;
	}
	return 0;
}

int register_guild_callback(struct mg_connection * conn, void * cbdata) {

	const struct mg_request_info * info = mg_get_request_info(conn);

	std::string path = info->local_uri;
	std::string guild = path.substr(std::strlen("/register/"));

	if (std::all_of(guild.begin(), guild.end(), [](const unsigned char & c) {return isdigit(c);})) {

		HTTPHeaders request_headers(info->http_headers, info->num_headers);

		std::string auth_token = authenticate(request_headers.cookies);

		if (!auth_token.empty()) {

			int64_t guild_id = std::stoll(guild);

			std::string entries;
			
			std::vector<Channel> channels = get_guild_channels(guild_id, BOT_TOKEN);
			
			pqxx::connection pgconn(CONN_STR);
			pqxx::work work(pgconn);
			work.exec("CREATE TABLE IF NOT EXISTS subscription(id SERIAL PRIMARY KEY, guild BIGINT, text_channel BIGINT, yt_channel VARCHAR(252))");
			pqxx::result table = work.exec_params("SELECT text_channel, yt_channel FROM subscription WHERE guild=$1", guild);

			std::vector<Guild> guilds = get_guilds(auth_token);

			auto it = std::find_if(guilds.begin(), guilds.end(), [guild_id](const Guild & guild){ return guild.id == guild_id; });

			Guild guild;

			if (it != guilds.end()) {
				guild = *it;
			}
			
			for (const pqxx::row & row : table) {
				int64_t text_channel = row[0].as<int64_t>();
				std::string yt_channel = row[1].as<std::string>();
				auto it = std::find_if(channels.begin(), channels.end(), [text_channel](const Channel & c) { return c.id == text_channel; });
				if (it == channels.end()) {
					pqxx::work(pgconn).exec_params("DELETE * FROM subscription WHERE text_channel=$1", std::to_string(text_channel));
				}
				else {
					entries += format(load_file(WEB_ROOT "register/<guild_id>/channel_frag.html"), { std::to_string(guild.id), std::to_string(text_channel), it->name, yt_channel });
				}
			}

			work.commit();

			std::string channels_options;

			for (const Channel & channel : channels) {
				channels_options += "<option value='" + std::to_string(channel.id) + "'>" + channel.name + "</option>";
			}

			HTTPHeaders response_headers;
			response_headers["Content-Type"] = "text/html; charset=utf-8";
			response_headers["Connection"] = "close";
			mg_printf(conn, "HTTP/1.1 200 OK\r\n%s", std::string(response_headers).c_str());
			mg_printf(conn, load_file(WEB_ROOT "register/<guild_id>/index.html").c_str(),
				guild.name.c_str(),
				guild.name.c_str(),
				std::to_string(guild.id).c_str(),
				entries.c_str(),
				channels_options.c_str()
			);

			return 1;
		}
		mg_send_http_redirect(conn, "/register/login/", 303);
		return 1;
	}
	return 0;
}

int register_callback(struct mg_connection * conn, void * cbdata) {

	const struct mg_request_info * info = mg_get_request_info(conn);
	HTTPHeaders request_headers(info->http_headers, info->num_headers);
	HTMLForm query = info->query_string;

	std::string auth_token = authenticate(request_headers.cookies);

	if (!auth_token.empty()) {

		std::string auth_header = "Authorization: Bearer " + auth_token;

		curl_slist * header = curl_slist_append(NULL, auth_header.c_str());

		std::string identify_url = "https://discord.com/api/v9/users/@me";

		GET(identify_url, header, NULL, NULL);

		picojson::value json;
		picojson::parse(json, input);
		std::string name;

		if (json.get("global_name").is<std::string>()) {
			name = json.get("global_name").to_str();
		}
		else if (json.get("username").is<std::string>()) {
			name = json.get("username").to_str();
		}
		else {
			name = "nobody";
		}

		HTTPHeaders response_headers;
		response_headers["Content-Type"] = "text/html; charset=utf-8";
		response_headers["Connection"] = "close";
		std::string table_rows;
		const std::vector<Guild> guilds = get_guilds(auth_token);
		for (const Guild & guild : guilds) {
			if (guild.permissions & (1 << 5)) {
				bool in_guild = bot_in_guild(guild.id);
				table_rows += format(load_file(WEB_ROOT "/register/guild_frag.html"), { std::to_string(guild.id), guild.name, in_guild ? "rem-bot" : "add-bot", in_guild ? "☑" : "☒" });
			}
		}
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\n%s", std::string(response_headers).c_str());
		mg_printf(conn, load_file(WEB_ROOT "/register/index.html").c_str(),
			name.c_str(),
			table_rows.c_str()
		);
		return 1;
	}
	mg_send_http_redirect(conn, "/register/login/", 303);
	return 1;
}

int oauth_callback(struct mg_connection * conn, void * cbdata) {

	const struct mg_request_info * info = mg_get_request_info(conn);

	HTMLForm query = info->query_string;

	if (query.has("code")) {
		// if (query.has("guild_id")) {
		// 	mg_send_http_redirect(conn, "/register/", 303);
		// 	return 1;
		// }

		std::string code = query["code"];

		HTMLForm data;
		data["client_id"] = CLIENT_ID;
		data["client_secret"] = CLIENT_SECRET;
		data["grant_type"] = "authorization_code";
		data["code"] = code;
		data["redirect_uri"] = REDIRECT_URL;
		output = data;

		output_type = "application/x-www-form-urlencoded";
		POST(TOKEN_URL, NULL, NULL, NULL);
		picojson::value json;
		picojson::parse(json, input);
		if (!(json.get("access_token").is<std::string>() && json.get("refresh_token").is<std::string>())) {
			mg_printf(conn,
				"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
				"close\r\n\r\n");
			mg_printf(conn, "<!DOCTYPE html><html><body>");
			mg_printf(conn, "<h1>Unauthorized</h1><p>%s</p>", input.c_str());
			mg_printf(conn, "</body></html>\n");
		}
		else {
			std::string uuid = uuid_generator.getUUID().str();
			std::string access_token = json.get("access_token").to_str();
			std::string refresh_token = json.get("refresh_token").to_str();
			pqxx::connection pqconn(CONN_STR);
			pqxx::work work(pqconn);
			std::string token_part = access_token.substr(0, access_token.find('.'));
			//pqxx::result table = work.exec("SELECT id FROM session WHERE access_token LIKE '" + pqconn.esc(std::string_view(token_part)) + "%';");
			//if (table.empty()) {
				work.exec_params("INSERT INTO session(id, access_token, refresh_token) VALUES($1, $2, $3);", uuid, access_token, refresh_token);
				printf("created new user %s\n", uuid.c_str());
			// }
			// else {
			// 	uuid = table[0][0].as<std::string>();
			// 	for (int i = 1; i < table.size(); i++) {
			// 		pqxx::result table2 = work.exec_params("DELETE FROM session WHERE id=$1 RETURNING access_token;", table[i][0].as<std::string>());
			// 		revoke_token(table[0][0].as<std::string>());
			// 	}
			// 	work.exec_params("UPDATE session SET access_token=$2, refresh_token=$3 WHERE id=$1;", uuid, access_token, refresh_token);
			// 	printf("found existing user %s\n", uuid.c_str());
			// }
			work.commit();
			
			std::string token = jwt::create()
				.set_type("JWT")
				.set_issuer("StreamBot")
				.set_payload_claim("User", jwt::claim(uuid))
				.sign(jwt::algorithm::hs256("streambot"));

			mg_printf(conn,
				"HTTP/1.1 303 See Other\r\n"
				"Location: /register/\r\n"
				"Content-Length: 0\r\n"
				"Content-Type: text/html\r\n"
				"Connection: close\r\n"
#ifdef _DEBUG
				"Set-Cookie: JWT=%s; Path=/register/; SameSite=Lax; Max-Age=%llu; HttpOnly\r\n"
#else
				"Set-Cookie: JWT=%s; Path=/register/; SameSite=Lax; Max-Age=%llu; Secure; HttpOnly\r\n"
#endif
				"\r\n"
				,
				token.c_str(),
				24llu * 3600llu);
		}
		return 1;
	}
	return 0;
}