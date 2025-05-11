#define _CRT_SECURE_NO_WARNINGS

#include "util.h"

#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#define min(a, b) (a) < (b) ? (a) : (b)
#endif

char * input = NULL;
size_t input_size = 0;
size_t input_cap = 0;

char * output = NULL;
size_t output_size = 0;
size_t output_cap = 0;
size_t output_ptr = 0;
char output_type[256] = { 0 };

CURL * curl = NULL;

#define SUB_TIMEOUT 10000000
#define SLEEP_TIME 100

static volatile char * sub_topic;

static volatile bool sub_wait;

static volatile long long sub_lease;

static bool areDigits(const char * str) {
	bool flag = true;
	for (char * c = str; *c && flag; c++) {
		flag = flag && isdigit(*c);
	}
	return flag;
}

char * get_token() {

	FILE * ftok = fopen("token.txt", "r");

	if (!ftok) return NULL;

	int n = 0;
	while (n < 1024 && !feof(ftok)) fgetc(ftok), n++;

	rewind(ftok);

	char * tok = malloc(n + 1);

	n = 0;
	while (!feof(ftok)) tok[n] = fgetc(ftok);
	tok[n] = 0;

	return tok;
}

size_t write_data(void * buffer, size_t size, size_t nmemb, void * _) {
	size_t bytes = size * nmemb;
	while (input_size + bytes > input_cap) {
		input = realloc(input, input_cap *= 2);
	}
	memmove(input + input_size, buffer, bytes);
	input_size += bytes;
	return bytes;
}

size_t read_data(void * buffer, size_t size, size_t nmemb, void * _) {
	size_t bytes = min(size * nmemb, output_size - output_ptr);
	memmove(buffer, output + output_ptr, bytes);
	output_ptr += bytes;
	return bytes;
}

CURLcode init_curl(void) {

	THROW_CURL(curl_global_init(CURL_GLOBAL_DEFAULT), {})

	curl = curl_easy_init();
	if (!curl) {
		curl_global_cleanup();
		return -1;
	}

	return CURLE_OK;

}

CURLcode GET(const char * url) {

	if (input) {
		input_size = 0;
		*input = 0;
	}
	else {
		input_cap = 16;
		input = malloc(input_cap);
	}

	curl_easy_reset(curl);

	THROW_CURL(curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem"), {});
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data), {});
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_URL, url), {});

	THROW_CURL(curl_easy_perform(curl), {});

	write_data("", 1, 1, NULL);

	return CURLE_OK;

}

CURLcode POST(const char * url, struct curl_slist * header) {

	if (input) {
		input_size = 0;
		*input = 0;
	}
	else {
		input_cap = 16;
		input = malloc(input_cap);
	}

	curl_easy_reset(curl);

	bool flag = !header;

	char * h = malloc(14 + strlen(output_type) + 1);
	sprintf(h, "Content-Type: %s", output_type);
	header = curl_slist_append(header, h);
	free(h);

	#define CLEANUP { if (flag) curl_slist_free_all(header); }

	THROW_CURL(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem"), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_data), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, output_size), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_POST, 1), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_URL, url), CLEANUP);

	output_ptr = 0;

	THROW_CURL(curl_easy_perform(curl), CLEANUP);

	write_data("", 1, 1, NULL);

	CLEANUP;

	return CURLE_OK;

	#undef CLEANUP

}

CURLcode Discord_POST(const char * endpoint, const char * token) {

	//if (input) {
	//	input_size = 0;
	//	*input = 0;
	//}
	//else {
	//	input_cap = 16;
	//	input = malloc(input_cap);
	//}

	//curl_easy_reset(curl);

	char * url = malloc(28 + strlen(endpoint) + 1); // strlne("https://discord.com/api/v10/");
	char * authorization = malloc(19 + strlen(token) + 1); // strlen("Authorization: Bot ");

	sprintf(url, "https://discord.com/api/v10/%s", endpoint);
	sprintf(authorization, "Authorization: Bot %s", token);

	struct curl_slist * header = NULL;
	header = curl_slist_append(header, authorization);
	sprintf(output_type, "application/json");
	//header = curl_slist_append(header, "Content-Type: application/json");

	#define CLEANUP { free(url); free(authorization); curl_slist_free_all(header); }

	THROW_CURL(POST(url, header), CLEANUP);

	//THROW_CURL(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header), CLEANUP);
	//THROW_CURL(curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem"), CLEANUP);
	//THROW_CURL(curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_data), CLEANUP);
	//THROW_CURL(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data), CLEANUP);
	//THROW_CURL(curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, output_size), CLEANUP);
	//THROW_CURL(curl_easy_setopt(curl, CURLOPT_POST, 1), CLEANUP);
	//THROW_CURL(curl_easy_setopt(curl, CURLOPT_URL, url), CLEANUP);

	//output_ptr = 0;

	//THROW_CURL(curl_easy_perform(curl), CLEANUP);

	//write_data("", 1, 1, NULL);

	CLEANUP;

	return CURLE_OK;

	#undef CLEANUP

}

char * poll_RSS(const char * url) {

	ERROR_CURL(GET(url), { return NULL; });

	char * entry = strstr(input, "<entry>");

	if (entry) {

		char * entry_end = strstr(input, "</entry>");

		if (entry_end) {
			entry_end += 8; // strlen("</entry>");
			*entry_end = 0;
		}

		return entry;
	}
	return NULL;
}

void subscribe_RSS(const char * url, long long * lease) {

	output = malloc(512);
	output_cap = 512;

	sprintf(output_type, "application/x-www-form-urlencoded");

	snprintf(output, 512, "hub.callback=https://3.141592.dev/subscriptioncallback&hub.mode=subscribe&hub.topic=%s", url);

	output_size = strlen(output);

	sub_topic = strdup(url);

	POST("https://pubsubhubbub.appspot.com/subscribe", NULL);

	sub_wait = true;

	long long timeout = SUB_TIMEOUT;

	while (sub_wait && timeout) {
		SLEEP(SLEEP_TIME);
		timeout -= SLEEP_TIME;
		if (timeout < 0) timeout = 0;
	}

	if (!timeout) {
		puts("Timed out subscribbing");
	}

	free(output);
	free(sub_topic);
	output = NULL;
	sub_topic = NULL;
	if (lease) *lease = sub_lease;
}

int confirm_subscription(struct mg_connection * conn, const struct mg_request_info * info) {

	struct {
		char * mode;
		char * topic;
		char * challenge;
		char * lease;
	} hub = { 0 };

	char * query = NULL;

	sub_lease = 0;

	if (info->query_string) {
		query = strdup(info->query_string);
	}
	else {
		query = strdup("");
	}

	char * tok = strtok(query, "&");

	while (tok) {
		if (strncmp(tok, "hub.mode", strlen("hub.mode")) == 0) {
			tok = strchr(tok, '=') + 1;
			if (tok == (char *)NULL + 1) break;
			hub.mode = tok;
		}
		else if (strncmp(tok, "hub.topic", strlen("hub.topic")) == 0) {
			tok = strchr(tok, '=') + 1;
			if (tok == (char *)NULL + 1) break;
			mg_url_decode(tok, strlen(tok) + 1, tok, strlen(tok) + 1, true);
			hub.topic = tok;
		}
		else if (strncmp(tok, "hub.challenge", strlen("hub.challenge")) == 0) {
			tok = strchr(tok, '=') + 1;
			if (tok == (char *)NULL + 1) break;
			hub.challenge = tok;
		}
		else if (strncmp(tok, "hub.lease_seconds", strlen("hub.lease_seconds")) == 0) {
			tok = strchr(tok, '=') + 1;
			if (tok == (char *)NULL + 1) break;
			hub.lease = tok;
		}
		tok = strtok(NULL, "&");
	}

	bool flag = sub_topic && hub.mode && hub.topic && hub.challenge && hub.lease;

	if (flag) {
		flag = flag
			&& (strcmp(hub.mode, "subscribe") == 0)
			&& (strcmp(hub.topic, sub_topic) == 0)
			&& areDigits(hub.lease);
	}

	if (!flag) {
		mg_printf(conn,
			"HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nConnection: "
			"close\r\n\r\n");
		mg_printf(conn, "Nothing to see here, this endpoint is for pubsubhubbub");
		free(query);
		return 1;
	}

	mg_printf(conn,
		"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: "
		"close\r\n\r\n");
	mg_printf(conn, hub.challenge);

	sub_lease = atoll(hub.lease);

	sub_wait = false;

	free(query);

	return 1;
}

bool is_new_entry(const char * entry_xml) {

	FILE * f = fopen("last_date.txt", "r");

	char old_date[64] = { 0 };
	char new_date[64] = { 0 };

	if (f) {
		for (int i = 0; i < 63; i++) {
			char c = fgetc(f);
			if (feof(f)) break;
			old_date[i] = c;
		}
		fclose(f);
	}

	char * date_xml = strstr(entry_xml, "<published>");
	if (!date_xml) return false;
	date_xml += 11; // strlen("<published>");
	char * date_xml_end = strstr(entry_xml, "</published>");
	if (!date_xml_end) return false;

	memcpy(new_date, date_xml, min(date_xml_end - date_xml, 63));

	bool new = strcmp(old_date, new_date) != 0;

	if (new) {
		f = fopen("last_date.txt", "w");
		fwrite(new_date, 1, strlen(new_date), f);
		fclose(f);
	}

	return new;
}
