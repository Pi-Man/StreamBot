#include "register.h"

#include <string.h>
#include <malloc.h>

#include "util.h"

#define AUTH_URL "https://discord.com/oauth2/authorize?client_id=1336495404308762685&response_type=code&redirect_uri=https%3A%2F%2F3.141592.dev%2Fregister&scope=guilds+guilds.members.read"
#define TOKEN_URL "https://discord.com/api/oauth2/token"
#define REDIRECT_URL "https://3.141592.dev/register"
#define CLIENT_ID "1336495404308762685"
#define CLIENT_SECRET "" //TODO: make file before committing

int register_callback(struct mg_connection * conn, void * cbdata) {

	const struct mg_request_info * info = mg_get_request_info(conn);

	info->query_string;

	const char * code = NULL;
	if (info->query_string) code = strstr(info->query_string, "code=");

	if (code) {
		code += 5;
		char * code_end = strchr(code, '&');
		if (code_end) *code_end = 0;
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
			"close\r\n\r\n");
		mg_printf(conn, "<!DOCTYPE html><html><body>");
		input_size = 0;
		input = malloc(16);
		input_cap = 16;
		write_data("{'grant_type':'authorization_code','code':'", 1, 43, NULL);
		write_data(code, 1, strlen(code), NULL);
		write_data("', 'redirect_uri':'"REDIRECT_URL"'}", 1, 50, NULL);
		if (output_cap < input_size) {
			output = realloc(output, input_size);
		}
		memcpy(output, input, input_size);
		free(input);
		output_size = input_size;
		sprintf(output_type, "application/x-www-form-urlencoded");
		POST(TOKEN_URL, NULL, CLIENT_ID, CLIENT_SECRET);
		mg_printf(conn, "<h1>Authorized</h1>");
		mg_printf(conn, "<pre><code>%s</code></pre>", input);
		mg_printf(conn, "</body></html>\n");
	}
	else {
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
			"close\r\n\r\n");
		mg_printf(conn, "<!DOCTYPE html><html><body>");
		mg_printf(conn, "%s", "<a href="AUTH_URL">Login</a>");
		mg_printf(conn, "</body></html>\n");
		return 1;
	}
}