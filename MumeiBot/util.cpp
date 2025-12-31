#define _CRT_SECURE_NO_WARNINGS

#include "util.h"

#include <fstream>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <sstream>
#include <regex>

#include <jwt-cpp/base.h>
#include <jwt-cpp/jwt.h>
#define PICOJSON_USE_INT64
#include <picojson/picojson.h>

#include "htmlform.h"

// #ifndef _WIN32
// #define min(a, b) (a) < (b) ? (a) : (b)
// #endif

std::string input;

std::string output;
std::string output_type;

CURL * curl = NULL;

#define SUB_TIMEOUT 10000
#define SLEEP_TIME 100

static std::mutex sub_mutex;

static std::string sub_topic;

static std::atomic<bool> sub_wait;

static long long sub_lease;

// static bool areDigits(const char * str) {
// 	bool flag = true;
// 	for (const char * c = str; *c && flag; c++) {
// 		flag = flag && isdigit(*c);
// 	}
// 	return flag;
// }

std::string load_file(const std::string & file_name) {

	static std::mutex mutex;

	std::lock_guard lock(mutex);

	std::string contents;

	std::ifstream file(file_name);

	if (file.good()) {
		while (!file.eof()) {
			int c = file.get();
			if (c != EOF) contents.push_back(c);
		}
	}
	contents.erase(contents.find_last_not_of("\n\r") + 1);
	return contents;
}

size_t write_data(void * buffer, size_t size, size_t nmemb, void * _) {
	size_t bytes = size * nmemb;
	input.append((const char*)buffer, bytes);
	return bytes;
}

size_t read_data(void * buffer, size_t size, size_t nmemb, void * _) {
	size_t bytes = std::min(size * nmemb, output.length());
	output.copy((char*)buffer, bytes);
	output.erase(0, bytes);
	return bytes;
}

CURLcode init_curl(void) {

	THROW_CURL(curl_global_init(CURL_GLOBAL_DEFAULT), {})

	curl = curl_easy_init();
	if (!curl) {
		curl_global_cleanup();
		return CURLE_FAILED_INIT;
	}

	return CURLE_OK;

}

CURLcode GET(const std::string & url, struct curl_slist * header, const char * username, const char * password) {

	input.clear();

	curl_easy_reset(curl);

	#define CLEANUP {}
	if (username) THROW_CURL(curl_easy_setopt(curl, CURLOPT_USERNAME, username), CLEANUP);
	if (username && password) THROW_CURL(curl_easy_setopt(curl, CURLOPT_PASSWORD, password), CLEANUP);
	if (header) THROW_CURL(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem"), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_URL, url.c_str()), CLEANUP);

	THROW_CURL(curl_easy_perform(curl), CLEANUP);

	CLEANUP;

	long http_code;

	THROW_CURL(curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code), {});

	return (CURLcode) http_code;

	#undef CLEANUP

}

CURLcode POST(const std::string & url, struct curl_slist * header, const char * username, const char * password) {

	input.clear();

	curl_easy_reset(curl);

	bool flag = !header;

	std::string content_header = "Content-Type: " + output_type;
	header = curl_slist_append(header, content_header.c_str());

	#define CLEANUP { if (flag) curl_slist_free_all(header); }
	if (username) THROW_CURL(curl_easy_setopt(curl, CURLOPT_USERNAME, username), CLEANUP);
	if (username && password) THROW_CURL(curl_easy_setopt(curl, CURLOPT_PASSWORD, password), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem"), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_data), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, output.length()), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_POST, 1), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_URL, url.c_str()), CLEANUP);

	THROW_CURL(curl_easy_perform(curl), CLEANUP);

	CLEANUP;

	return CURLE_OK;

	#undef CLEANUP

}

CURLcode DELETE(const std::string & url, struct curl_slist * header, const char * username, const char * password) {

	input.clear();

	curl_easy_reset(curl);

	bool flag = !header && !output.empty();

	if (!output.empty()) {
		std::string content_header = "Content-Type: " + output_type;
		header = curl_slist_append(header, content_header.c_str());
	}

	#define CLEANUP { if (flag) curl_slist_free_all(header); }
	if (username) THROW_CURL(curl_easy_setopt(curl, CURLOPT_USERNAME, username), CLEANUP);
	if (username && password) THROW_CURL(curl_easy_setopt(curl, CURLOPT_PASSWORD, password), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem"), CLEANUP);
	if (!output.empty()) THROW_CURL(curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_data), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data), CLEANUP);
	if (!output.empty()) THROW_CURL(curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, output.length()), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE"), CLEANUP);
	THROW_CURL(curl_easy_setopt(curl, CURLOPT_URL, url.c_str()), CLEANUP);

	THROW_CURL(curl_easy_perform(curl), CLEANUP);

	CLEANUP;

	return CURLE_OK;

	#undef CLEANUP

}

CURLcode Discord_POST(const std::string & endpoint, const std::string & token) {

	//if (input) {
	//	input_size = 0;
	//	*input = 0;
	//}
	//else {
	//	input_cap = 16;
	//	input = malloc(input_cap);
	//}

	//curl_easy_reset(curl);

	std::string url = "https://discord.com/api/v10/" + endpoint;
	std::string authorization = "Authorization: Bot " + token;

	struct curl_slist * header = NULL;
	header = curl_slist_append(header, authorization.c_str());
	output_type = "application/json";

	#define CLEANUP { curl_slist_free_all(header); }

	THROW_CURL(POST(url, header, NULL, NULL), CLEANUP);

	CLEANUP;

	return CURLE_OK;

	#undef CLEANUP

}

std::string get_body(struct mg_connection * conn) {

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

	return data;
}

// char * poll_RSS(const char * url) {

// 	ERROR_CURL(GET(url), { return NULL; });

// 	char * entry = strstr(input, "<entry>");

// 	if (entry) {

// 		char * entry_end = strstr(input, "</entry>");

// 		if (entry_end) {
// 			entry_end += 8; // strlen("</entry>");
// 			*entry_end = 0;
// 		}

// 		return entry;
// 	}
// 	return NULL;
// }

void subscribe_RSS(const std::string & url, const std::string & query_params, long long * lease) {

	output_type = "application/x-www-form-urlencoded";

	HTMLForm hubform;
	hubform["hub.callback"] = "https://" HOST "/subscriptioncallback?" + query_params;
	hubform["hub.mode"] = "subscribe";
	hubform["hub.topic"] = url;
	output = hubform;

	sub_mutex.lock();
	sub_topic = url;
	sub_wait = true;
	sub_mutex.unlock();

	POST("https://pubsubhubbub.appspot.com/subscribe", NULL, NULL, NULL);

	puts(input.c_str());

	long long timeout = SUB_TIMEOUT;

	while (sub_wait && timeout) {
		SLEEP(SLEEP_TIME);
		timeout -= SLEEP_TIME;
		if (timeout < 0) timeout = 0;
	}

	if (!timeout) {
		puts("Timed out subscribbing");
	}

	output = "";
	sub_mutex.lock();
	sub_topic = "";
	if (lease) *lease = !timeout ? 0 : sub_lease;

	sub_mutex.unlock();
}

int confirm_subscription(struct mg_connection * conn, const struct mg_request_info * info) {

	std::lock_guard lock(sub_mutex);

	sub_lease = 0;

	HTMLForm query_form = info->query_string;

	puts(info->local_uri_raw);

	bool flag = 
		!sub_topic.empty() && 
		query_form.has("hub.mode") && 
		query_form.has("hub.topic") && 
		query_form.has("hub.challenge") && 
		query_form.has("hub.lease_seconds");

	if (flag) {
		flag = 
			query_form["hub.mode"] == "subscribe" && 
			query_form["hub.topic"] == sub_topic && 
			std::all_of(query_form["hub.lease_seconds"].begin(), query_form["hub.lease_seconds"].end(), isdigit);
	}

	if (!flag) {
		mg_printf(conn,
			"HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nConnection: "
			"close\r\n\r\n");
		mg_printf(conn, "Nothing to see here, this endpoint is for pubsubhubbub");
		return 1;
	}

	mg_printf(conn,
		"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: "
		"close\r\n\r\n");
	mg_printf(conn, "%s", query_form["hub.challenge"].c_str());

	sub_lease = atoll(query_form["hub.lease_seconds"].c_str());

	sub_wait = false;

	return 1;
}

std::string get_link(const std::string &body) {
	std::regex regex(R"-(<link rel="alternate" href="(https://(?:www)\.youtube\.com/watch\?v=)-" YT_VIDEO_ID R"-()"/>)-");
    std::smatch match;
	if (std::regex_search(body, match, regex)) {
		return match[1];
	}
	return std::string();
}

std::string format(const std::string & format, const std::initializer_list<const std::string> && args) {
    std::string out;
	out.reserve(format.length());

	ptrdiff_t argc = args.end() - args.begin();

	for (int i = 0; i < format.length(); i++) {
		unsigned char c = format[i];
		if (c == '%') {
			i++;
			c = format[i];
			ptrdiff_t index = 0;
			if (c == '%') {
				out.push_back(c);
			}
			else if (isdigit(c)) {
				index = c - '0';
				i++;
				while (isdigit(c = format[i])) {
					index *= 10;
					index += c - '0';
					i++;
				}
				i--;
				if (index < argc) {
					const std::string & str = args.begin()[index];
					out.reserve(out.capacity() + str.length());
					out += str;
				}
			}
		}
		else {
			out.push_back(c);
		}
	}

	return out;
}

std::pair<std::string, std::string> get_user_hash_basicauth(const HTTPHeaders &headers) {

	std::regex auth_regex(R"(Basic ([a-zA-Z0-9+/]))");
	std::smatch match;
	if (std::regex_match(headers["Authorization"], match, auth_regex)) {
		std::string auth = jwt::base::decode<jwt::alphabet::base64>(match[1].str());
		size_t index = auth.find(':');
		std::string user = auth.substr(0, index);
		std::string pass = auth.substr(index + 1);
		static jwt::algorithm::hs512 hasher("pirpasshash");
		std::error_code err;
		std::string hash = hasher.sign(pass, err);
		if (!err) {
			return { user, hash };
		}
	}

    return { "", "" };
}

// bool is_new_entry(const char * entry_xml) {

// 	FILE * f = fopen("last_date.txt", "r");

// 	char old_date[64] = { 0 };
// 	char new_date[64] = { 0 };

// 	if (f) {
// 		for (int i = 0; i < 63; i++) {
// 			char c = fgetc(f);
// 			if (feof(f)) break;
// 			old_date[i] = c;
// 		}
// 		fclose(f);
// 	}

// 	const char * date_xml = strstr(entry_xml, "<published>");
// 	if (!date_xml) return false;
// 	date_xml += 11; // strlen("<published>");
// 	const char * date_xml_end = strstr(entry_xml, "</published>");
// 	if (!date_xml_end) return false;

// 	memcpy(new_date, date_xml, min(date_xml_end - date_xml, 63));

// 	bool is_new = strcmp(old_date, new_date) != 0;

// 	if (is_new) {
// 		f = fopen("last_date.txt", "w");
// 		fwrite(new_date, 1, strlen(new_date), f);
// 		fclose(f);
// 	}

// 	return is_new;
// }
