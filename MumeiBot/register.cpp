#include "register.h"

#include <string>
#include <mutex>

#include <jwt-cpp/jwt.h>
#include <uuid_v4.h>

#include "util.h"
#include "htmlform.h"
#include "httpheaders.h"

#ifdef _DEBUG
#define AUTH_URL "https://discord.com/oauth2/authorize?client_id=1336495404308762685&response_type=code&redirect_uri=http%3A%2F%2Flocalhost%3A65000%2Foauth%2Fdiscord%2Fcallback&scope=identify+guilds+guilds.members.read"
#define REDIRECT_URL "http://localhost:65000/oauth/discord/callback"
#else
#define AUTH_URL "https://discord.com/oauth2/authorize?client_id=1336495404308762685&response_type=code&redirect_uri=https%3A%2F%2F3.141592.dev%2Foauth%2Fdiscord%2Fcallback&scope=identify+guilds+guilds.members.read"
#define REDIRECT_URL "https://3.141592.dev/oauth/discord/callback"
#endif
#define TOKEN_URL "https://discord.com/api/oauth2/token"
#define CLIENT_ID "1336495404308762685"
#define CLIENT_SECRET load_file("secret.txt")

static UUIDv4::UUIDGenerator<std::mt19937_64> uuid_generator;
static std::mutex usermap_mutex;
static std::unordered_map<std::string, std::pair<std::string, std::string>> usermap;
static std::unordered_map<std::string, std::string> r_usermap;

int login_callback(struct mg_connection * conn, void * cbdata) {
	HTTPHeaders response_headers;
	response_headers["Content-Type"] = "text/html; charset=utf-8";
	response_headers["Connection"] = "close";
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n%s", std::string(response_headers).c_str());
	mg_printf(conn, load_file(WEB_ROOT "login/index.html").c_str(), AUTH_URL);
	return 1;
}

int logout_callback(struct mg_connection * conn, void * cbdata) {

	const struct mg_request_info * info = mg_get_request_info(conn);
	HTTPHeaders request_headers(info->http_headers, info->num_headers);
	if (request_headers.cookies.has("JWT")) {
		jwt::decoded_jwt jwt = jwt::decode(request_headers.cookies["JWT"]);
		jwt::claim user = jwt.get_payload_claim("User");
		jwt::verifier verifier = jwt::verify()
			.with_type("JWT")
			.with_issuer("StreamBot")
			.with_claim("User", user)
			.allow_algorithm(jwt::algorithm::hs256("streambot"));
		std::error_code error;
		verifier.verify(jwt, error);
		usermap_mutex.lock();
		std::string auth_token = usermap.find(user.as_string()) == usermap.end() ? "" : usermap[user.as_string()].first;
		usermap_mutex.unlock();
		if (!error && !auth_token.empty()) {
			usermap.erase(user.as_string());
			r_usermap.erase(auth_token);
			HTMLForm form;
			form["token"] = auth_token;
			form["token_type_hint"] = "access_token";
			form["client_id"] = CLIENT_ID;
			form["client_secret"] = CLIENT_SECRET;
			output = form;
			output_type = "application/x-www-form-urlencoded";
			POST(TOKEN_URL "/revoke", NULL, NULL, NULL);
			puts(input.c_str());
		}
	}

	HTTPHeaders response_headers;
	response_headers["Content-Type"] = "text/html; charset=utf-8";
	response_headers["Clear-Site-Data: "] = "\"cookies\"";
	response_headers["Connection"] = "close";
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n%s", std::string(response_headers).c_str());
	mg_printf(conn, "<!DOCTYPE html><title>Register New Link</title><html><body>");
	mg_printf(conn, "<p>Goodbye!</p>");
	mg_printf(conn, "</body></html>");

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
	html << "<select name='guild' id='guild'>";
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

int register_callback(struct mg_connection * conn, void * cbdata) {

	const struct mg_request_info * info = mg_get_request_info(conn);
	HTTPHeaders request_headers(info->http_headers, info->num_headers);
	HTMLForm query = info->query_string;

	if (request_headers.cookies.has("JWT")) {
		jwt::decoded_jwt jwt = jwt::decode(request_headers.cookies["JWT"]);
		jwt::claim user = jwt.get_payload_claim("User");
		jwt::verifier verifier = jwt::verify()
			.with_type("JWT")
			.with_issuer("StreamBot")
			.with_claim("User", user)
			.allow_algorithm(jwt::algorithm::hs256("streambot"));
		std::error_code error;
		verifier.verify(jwt, error);
		usermap_mutex.lock();
		std::string auth_token = usermap.find(user.as_string()) == usermap.end() ? "" : usermap[user.as_string()].first;
		usermap_mutex.unlock();
		if (!error && !auth_token.empty()) {

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
			mg_printf(conn,
				"HTTP/1.1 200 OK\r\n%s", std::string(response_headers).c_str());
			mg_printf(conn, "<!DOCTYPE html><title>Register New Link</title><html><body>");
			mg_printf(conn, "<p>Welcome %s!</p>", name.c_str());

			if (query.has("guild")) {
				int64_t guild_id = atoll(query["guild"].c_str());
				mg_printf(conn, "%s", get_channels_form(guild_id, load_file("token.txt")).c_str());
				mg_printf(conn, "<p>%s</p>", input.c_str());
			}
			else {
				mg_printf(conn, "%s", get_guild_form(auth_token).c_str());
			}

			mg_printf(conn, "<a href='/register/logout/'>Logout</a>");
			mg_printf(conn, "</body></html>");
			return 1;
		}
	}
	mg_send_http_redirect(conn, "/register/login/", 303);
	return 1;
}

int oauth_callback(struct mg_connection * conn, void * cbdata) {

	const struct mg_request_info * info = mg_get_request_info(conn);

	HTMLForm query = info->query_string;

	if (query.has("code")) {
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
			usermap_mutex.lock();
			std::string uuid;
			std::string access_token = json.get("access_token").to_str();
			std::string refresh_token = json.get("refresh_token").to_str();

			if (r_usermap.find(access_token) != r_usermap.end()) {
				uuid = r_usermap[access_token];
				printf("found exisiting user %s\n", uuid.c_str());
			}
			else {
				uuid = uuid_generator.getUUID().str();
				usermap[uuid] = { access_token, refresh_token };
				r_usermap[access_token] = uuid.c_str();
				printf("created new user %s\n", uuid.c_str());
			}
				
			usermap_mutex.unlock();

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