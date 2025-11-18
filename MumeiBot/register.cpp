#include "register.h"

#include <string>
#include <unordered_map>
#include <fstream>
//#include <sstream>

#include <string.h>
#include <malloc.h>

extern "C" {
#include "util.h"
}

#define AUTH_URL "https://discord.com/oauth2/authorize?client_id=1336495404308762685&response_type=code&redirect_uri=https%3A%2F%2F3.141592.dev%2Foauth%2Fdiscord%2Fcallback&scope=guilds+guilds.members.read+guilds.channels.read"
#define TOKEN_URL "https://discord.com/api/oauth2/token"
#define REDIRECT_URL "https://3.141592.dev/oauth/discord/callback"
#define CLIENT_ID "1336495404308762685"

std::unordered_map<std::string, std::string> parse_form(const std::string & form) {
	std::unordered_map<std::string, std::string> fields;
	size_t index = 0;
	for (size_t i = 0; i < form.length(); i++) {
		if (form[i] == '&') {
			size_t j = form.find('=', index);
			std::string key = form.substr(index, j - index);
			std::string val = form.substr(j + 1, i - j - 1);
			fields[key] = val;
		}
	}
	size_t j = form.find('=', index);
	std::string key = form.substr(index, j - index);
	std::string val = form.substr(j + 1);
	fields[key] = val;
	return fields;
}

std::string build_form(const std::unordered_map<std::string, std::string> & fields) {
	std::string form;
	char buffer[1024];
	for (const std::pair<std::string, std::string> & pair : fields) {
		mg_url_encode(pair.first.c_str(), buffer, 1024);
		form += buffer;
		form += "=";
		mg_url_encode(pair.second.c_str(), buffer, 1024);
		form += buffer;
		form += "&";
	}
	form.pop_back();
	return form;
}

int oauth_callback(struct mg_connection * conn, void * cbdata) {

	const struct mg_request_info * info = mg_get_request_info(conn);

	std::unordered_map<std::string, std::string> query = parse_form(info->query_string);

	if (query.find("code") != query.end()) {
		std::string code = query["code"];
		std::ifstream secret_file("secret.txt");
		std::string CLIENT_SECRET;
		secret_file >> CLIENT_SECRET;
		
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
			"close\r\n\r\n");
		mg_printf(conn, "<!DOCTYPE html><html><body>");

		std::unordered_map<std::string, std::string> data;
		data["client_id"] = CLIENT_ID;
		data["client_secret"] = CLIENT_SECRET;
		data["grant_type"] = "authorization_code";
		data["code"] = code;
		data["redirect_uri"] = REDIRECT_URL;
		std::string form = build_form(data);

		output = (char*) malloc(form.length());
		memcpy(output, form.c_str(), form.length());
		output_size = output_cap = form.length();
		sprintf(output_type, "application/x-www-form-urlencoded");
		POST(TOKEN_URL, NULL, NULL, NULL);
		mg_printf(conn, "<h1>Authorized</h1>");
		std::string json = input;
		size_t start = json.find("\"expires_in\"");
		size_t end = json.find(',', start);
		mg_printf(conn, "<pre><code>%s</code></pre>", json.substr(start, end - start).c_str());
		mg_printf(conn, "</body></html>\n");
		return 1;
	}
	else {
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
			"close\r\n\r\n");
		mg_printf(conn, "<!DOCTYPE html><html><body>");
		mg_printf(conn, "%s", "<a href=" AUTH_URL ">Login</a>");
		mg_printf(conn, "</body></html>\n");
		return 1;
	}
}