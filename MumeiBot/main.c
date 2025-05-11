#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "civetweb.h"

#include "util.h"

char * find_json_end(char * json);

int dynamic_page_request(struct mg_connection * conn, void * cbdata);

int subscription_page_request(struct mg_connection * conn, void * cbdata);

int redirect_to(struct mg_connection * conn, void * cbdata);

int main(void) {

	struct mg_callbacks callbacks = { 0 };
	const char * options[] = {
		#ifdef _DEBUG
		"document_root", "./webserver/root/",
		#else
		"document_root", "/home/ubuntu/webserver/root/",
		#endif
		"enable_directory_listing", "no",
		"static_file_max_age", "0",
		"enable_auth_domain_check", "no",
		"listening_ports", "localhost:65000",
		"request_timeout_ms", "10000",
		"access_log_file", "access.log",
		"error_log_file", "error.log",
		NULL };

	struct mg_context * ctx = mg_start(&callbacks, NULL, options);

	mg_set_request_handler(ctx, "/dynamic/$", dynamic_page_request, NULL);
	mg_set_request_handler(ctx, "/subscriptioncallback", subscription_page_request, NULL);

	FATAL_CURL(init_curl());

	long long lease = 0;

	while (1) {
		#ifndef _DEBUG
		subscribe_RSS("https://www.youtube.com/xml/feeds/videos.xml?channel_id=UCcHHkJ98eSfa5aj0mdTwwLQ", &lease);
		#endif

		SLEEP(lease * 1000);

	}

	const char * token = get_token();

	while (1) {

		const char * entry = poll_RSS("https://www.youtube.com/feeds/videos.xml?channel_id=UC3n5uGu18FoCy23ggWWp8tA");

		if (is_new_entry(entry)) {
			puts(entry);

			const char * link_start = strstr(entry, "<link rel=\"alternate\" href=\"");

			if (link_start) {
				link_start += 28; // strlen("<link rel=\"alternate\" href=\"");
				const char * link_end = strstr(link_start, "\"");

				const char * endpoint = "channels/599365997920649216/messages";

				output = malloc(256);
				output_cap = 256;
				sprintf(output, "{\"content\": \"%.*s\"}", (int)(link_end - link_start), link_start);
				output_size = strlen(output);

				//char * url = malloc(link_end - link_start + 1);
				//strncpy(url, link_start, link_end - link_start);
				//url[link_end - link_start] = 0;

				//GET(url);

				//char * json = strstr(input, "ytInitialData = ") + 16;
				//char * json_end = find_json_end(json);
				//*json_end = 0;

				Discord_POST(endpoint, token);

				free(output);

				//puts(json);
			}
		}

		SLEEP(60 * 5);

	}

	return 0;
}

char * find_json_end(char * json) {
	size_t count = 0;
	if (*json != '{') return NULL;
	do {
		if (*json == '{') count++;
		if (*json == '}') count--;
		json++;
	} while (count);
	return json;
}

volatile int count = 0;

int dynamic_page_request(struct mg_connection * conn, void * cbdata) {
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
		"close\r\n\r\n");
	mg_printf(conn, "<!DOCTYPE html><html><body>");
	mg_printf(conn, "<h2>This is a counter.</h2>");
	mg_printf(conn, "<p>%d</p>", count++);
	mg_printf(conn, "</body></html>\n");
	return 1;
}

int subscription_page_request(struct mg_connection * conn, void * cbdata) {
	const struct mg_request_info * info = mg_get_request_info(conn);

	if (strcmp(info->request_method, "GET") == 0) {
		return confirm_subscription(conn, info);
	}
	else if (strcmp(info->request_method, "POST") == 0) {
		char buffer[256];
		int bytes = 0;
		input_size = 0;
		do {
			bytes = mg_read(conn, buffer, 256);
			write_data(buffer, 1, bytes, NULL);
		} while (bytes == 256);
		mg_printf(conn,
			"HTTP/1.1 204 No Content\r\nConnection: "
			"close\r\n\r\n");
		puts(input);
	}

	return 1;
}

int redirect_to(struct mg_connection * conn, void * cbdata) {
	return mg_send_http_redirect(conn, (const char *)cbdata, 303);
}
