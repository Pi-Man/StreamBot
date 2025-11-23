#include "register.h"

#include <string>

#include <jwt-cpp/jwt.h>

#include "util.h"
#include "htmlform.h"
#include "httpheaders.h"

#define AUTH_URL "https://discord.com/oauth2/authorize?client_id=1336495404308762685&response_type=code&redirect_uri=https%3A%2F%2F3.141592.dev%2Foauth%2Fdiscord%2Fcallback&scope=guilds+guilds.members.read+guilds.channels.read"
#define TOKEN_URL "https://discord.com/api/oauth2/token"
#define REDIRECT_URL "https://3.141592.dev/oauth/discord/callback"
#define CLIENT_ID "1336495404308762685"

int register_callback(struct mg_connection * conn, void * cbdata) {

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
		if (!error) {
			HTTPHeaders response_headers;
			response_headers["Content-Type"] = "text/html";
			response_headers["Connection"] = "close";
			mg_printf(conn,
				"HTTP/1.1 200 OK\r\n%s", std::string(response_headers).c_str());
			mg_printf(conn, "<!DOCTYPE html><html><body>");
			mg_printf(conn, "<p>Welcome %%User%%!</p>");
			mg_printf(conn, "</body></html>\n");
			return 1;
		}
		mg_send_http_redirect(conn, "/login/", 303);
		return 1;
	}
}

int oauth_callback(struct mg_connection * conn, void * cbdata) {

	const struct mg_request_info * info = mg_get_request_info(conn);

	HTMLForm query = info->query_string;

	if (query.has("code")) {
		std::string code = query["code"];
		std::string CLIENT_SECRET = load_file("secret.txt");

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
		if (!json.contains("access_token")) {
			mg_printf(conn,
				"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
				"close\r\n\r\n");
			mg_printf(conn, "<!DOCTYPE html><html><body>");
			mg_printf(conn, "<h1>Unauthorized</h1>");
			mg_printf(conn, "</body></html>\n");
		}
		else {
			std::string token = jwt::create()
				.set_type("JWT")
				.set_issuer("StreamBot")
				.set_payload_claim("User", jwt::claim(std::string("1234")))
				.sign(jwt::algorithm::hs256("streambot"));

			mg_printf(conn,
				"HTTP/1.1 303 See Other\r\n"
				"Location: /register/\r\n"
				"Content-Length: 0\r\n"
				"Content-Type: text/html\r\n"
				"Connection: close\r\n"
				"Set-Cookie: JWT=%s; Domain=3.141592.dev; Path=/register/; SameSite=Lax; Max-Age=%llu; Secure; HttpOnly\r\n"
				"\r\n"
				,
				token.c_str(),
				24llu * 3600llu);
			
		}
		return 1;
	}
	return 0;
}