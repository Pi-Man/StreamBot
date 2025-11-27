#ifndef _H_UTIL
#define _H_UTIL

#include <string>
#include <vector>

#include <curl/curl.h>

#include "civetweb.h"
#include "guild.h"

#ifdef _WIN32
//#include <Windows.h>
#define SLEEP(milli) Sleep(milli);
#define strdup _strdup
#else
#include <unistd.h>
#include <time.h>
#define SLEEP(milli) { struct timespec ts; ts.tv_sec = milli / 1000; ts.tv_nsec = (milli % 1000) * 1000000; nanosleep(&ts, NULL); }
#endif

#ifdef _DEBUG
#define WEB_ROOT "./webserver/root/"
#else
#define WEB_ROOT "/home/ubuntu/webserver/root/"
#endif

extern std::string input;

extern std::string output;
extern std::string output_type;


#define FATAL_CURL(cmd) { CURLcode err = cmd; if (err) { fprintf(stderr, "Fatal Error: %s returned %d \"%s\"", #cmd, err, curl_easy_strerror(err)); exit(err); } }
#define ERROR_CURL(cmd, on_err) { CURLcode err = cmd; if (err) { fprintf(stderr, "Error: %s returned %d \"%s\"", #cmd, err, curl_easy_strerror(err)); on_err } }
#define THROW_CURL(cmd, finally) { CURLcode err = cmd; if (err) { finally; return err; } }
#define WARN_CURL(cmd) { CURLcode err = cmd; if (err) { fprintf(stderr, "WARNING: %s returned %d \"%s\"", #cmd, err, curl_easy_strerror(err)); } }

std::string load_file(const std::string & file_name);

size_t write_data(void * buffer, size_t size, size_t nmemb, void * _);

size_t read_data(void * buffer, size_t size, size_t nmemb, void * _);

CURLcode init_curl();

CURLcode GET(const std::string & url, struct curl_slist * header, const char * username, const char * password);

CURLcode POST(const std::string & url, struct curl_slist * header, const char * username, const char * password);

CURLcode Discord_POST(const std::string & endpoint, const std::string & token);

//char * poll_RSS(const char * url);

void subscribe_RSS(const std::string & url, long long * lease);

int confirm_subscription(struct mg_connection * conn, const struct mg_request_info * query);

std::vector<Guild> get_guilds(const std::string & auth_token);

//bool is_new_entry(const char * entry_xml);

#endif
