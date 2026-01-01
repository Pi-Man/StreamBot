#ifndef _H_UTIL
#define _H_UTIL

#include <string>
#include <vector>

#include <curl/curl.h>
#include <jwt-cpp/jwt.h>

#include "civetweb.h"
#include "guild.h"
#include "channel.h"
#include "httpheaders.h"

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
#define HOST "localhost:65000"
#else
#define WEB_ROOT "/home/ubuntu/webserver/root/"
#define HOST "3.141592.dev"
#endif
#define DISCORD_API "https://discord.com/api/v9/"
#define DISCORD_AUTH "https://discord.com/oauth2/authorize"
#ifdef _DEBUG
#define AUTH_URL "https://discord.com/oauth2/authorize?client_id=1336495404308762685&response_type=code&redirect_uri=http%3A%2F%2Flocalhost%3A65000%2Fregister%2Foauth%2Fdiscord%2Fcallback&scope=identify+guilds"
#define REDIRECT_URL "http://localhost:65000/register/oauth/discord/callback"
#define BOT_REDIRECT "http://localhost:65000/register/"
#else
#define AUTH_URL "https://discord.com/oauth2/authorize?client_id=1336495404308762685&response_type=code&redirect_uri=https%3A%2F%2F3.141592.dev%2Fregister%2Foauth%2Fdiscord%2Fcallback&scope=identify+guilds"
#define REDIRECT_URL "https://3.141592.dev/register/oauth/discord/callback"
#define BOT_REDIRECT "https://3.141592.dev/register/"
#endif
#define TOKEN_URL "https://discord.com/api/oauth2/token"
#define CLIENT_ID "1336495404308762685"
#define CLIENT_SECRET load_file("secret.txt")
#define BOT_TOKEN load_file("token.txt")

#define CONN_STR "host=localhost port=65001 user=postgres password=postgres"

#define YT_CHANNEL_ID R"(UC[A-Za-z0-9_-]{21}[AQgw])"
#define YT_VIDEO_ID R"([A-Za-z0-9_-]{10}[AEIMQUYcgkosw048])"

extern std::string input;

extern std::string output;
extern std::string output_type;

extern const jwt::algorithm::hs512 hasher;

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

CURLcode DELETE(const std::string & url, struct curl_slist * header, const char * username, const char * password);

CURLcode Discord_POST(const std::string & endpoint, const std::string & token);

std::string get_body(struct mg_connection * conn);

//char * poll_RSS(const char * url);

void subscribe_RSS(const std::string & url, const std::string & query_params, long long * lease);

int confirm_subscription(struct mg_connection * conn, const struct mg_request_info * query);

std::string get_link(const std::string & body);

std::string format(const std::string & format, const std::initializer_list<const std::string> && args);

//bool is_new_entry(const char * entry_xml);

std::pair<std::string, std::string> get_user_hash_basicauth(const HTTPHeaders & headers);

std::string bstos (const std::string & byte_string);

std::string stobs (const std::string & string);

#endif
