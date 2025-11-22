#include "register.h"

#include <string>
#include <unordered_map>
#include <fstream>
//#include <sstream>

#include <string.h>
#include <malloc.h>

#include "util.h"
#include "htmlform.h"

#define AUTH_URL "https://discord.com/oauth2/authorize?client_id=1336495404308762685&response_type=code&redirect_uri=https%3A%2F%2F3.141592.dev%2Foauth%2Fdiscord%2Fcallback&scope=guilds+guilds.members.read+guilds.channels.read"
#define TOKEN_URL "https://discord.com/api/oauth2/token"
#define REDIRECT_URL "https://3.141592.dev/oauth/discord/callback"
#define CLIENT_ID "1336495404308762685"

int oauth_callback(struct mg_connection * conn, void * cbdata) {

	const struct mg_request_info * info = mg_get_request_info(conn);

	HTMLForm query = info->query_string;

	if (query.has("code")) {
		std::string code = query["code"];
		std::string CLIENT_SECRET = load_file("secret.txt");
		
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
			"close\r\n\r\n");
		mg_printf(conn, "<!DOCTYPE html><html><body>");

		HTMLForm data;
		data["client_id"] = CLIENT_ID;
		data["client_secret"] = CLIENT_SECRET;
		data["grant_type"] = "authorization_code";
		data["code"] = code;
		data["redirect_uri"] = REDIRECT_URL;
		output = data;

		output_type = "application/x-www-form-urlencoded";
		POST(TOKEN_URL, NULL, NULL, NULL);
		std::string json = input;
		size_t start = json.find("\"expires_in\"");
		size_t end = json.find(',', start);
		if (start == std::string::npos) {
			mg_printf(conn, "<h1>Unauthorized</h1>");
			mg_printf(conn, "</body></html>\n");
		}
		else {
			mg_printf(conn, "<h1>Authorized</h1>");
			mg_printf(conn, "<pre><code>%s</code></pre>", json.substr(start, end - start).c_str());
			mg_printf(conn, "</body></html>\n");
		}
		return 1;
	}
	return 0;
}