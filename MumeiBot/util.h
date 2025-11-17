#ifndef _H_UTIL
#define _H_UTIL

#include <stdbool.h>

#include <curl/curl.h>

#include "civetweb.h"

#ifdef _WIN32
//#include <Windows.h>
#define SLEEP(milli) Sleep(milli);
#define strdup _strdup
#else
#include <unistd.h>
#include <time.h>
#define SLEEP(milli) { struct timespec ts; ts.tv_sec = milli / 1000; ts.tv_nsec = (milli % 1000) * 1000000; nanosleep(&ts, NULL); }
#endif

extern char * input;
extern size_t input_size;
extern size_t input_cap;

extern char * output;
extern size_t output_size;
extern size_t output_cap;
extern char output_type[256];


#define FATAL_CURL(cmd) { CURLcode err = cmd; if (err) { fprintf(stderr, "Fatal Error: %s returned %d \"%s\"", #cmd, err, curl_easy_strerror(err)); exit(err); } }
#define ERROR_CURL(cmd, on_err) { CURLcode err = cmd; if (err) { fprintf(stderr, "Error: %s returned %d \"%s\"", #cmd, err, curl_easy_strerror(err)); on_err } }
#define THROW_CURL(cmd, finally) { CURLcode err = cmd; if (err) { finally; return err; } }
#define WARN_CURL(cmd) { CURLcode err = cmd; if (err) { fprintf(stderr, "WARNING: %s returned %d \"%s\"", #cmd, err, curl_easy_strerror(err)); } }

char * get_token();

size_t write_data(void * buffer, size_t size, size_t nmemb, void * _);

size_t read_data(void * buffer, size_t size, size_t nmemb, void * _);

CURLcode init_curl(void);

CURLcode GET(const char * url);

CURLcode POST(const char * url, struct curl_slist * header, const char * username, const char * password);

CURLcode Discord_POST(const char * endpoint, const char * token);

char * poll_RSS(const char * url);

void subscribe_RSS(const char * url, long long * lease);

int confirm_subscription(struct mg_connection * conn, const struct mg_request_info * query);

bool is_new_entry(const char * entry_xml);

#endif
