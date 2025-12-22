#include "discord.h"

#include <sstream>

#include <curl/curl.h>
#include <picojson/picojson.h>

#include "util.h"

std::string get_user_name(const std::string &access_token) {

    std::string auth_header = "Authorization: Bearer " + access_token;

    curl_slist * header = curl_slist_append(NULL, auth_header.c_str());

    std::string identify_url = "https://discord.com/api/v9/users/@me";

    GET(identify_url, header, NULL, NULL);

    picojson::value json;
    picojson::parse(json, input);
    std::string name;

    if (json.get("global_name").is<std::string>()) {
        name = json.get("global_name").to_str();
    }
    else if (json.get("username").is<std::string>()) {
        name = json.get("username").to_str();
    }
    else {
        name = "nobody";
    }

    return name;
}

std::vector<Guild> get_user_guilds(const std::string & access_token) {

    std::vector<Guild> guilds;

	std::string auth_header = "Authorization: Bearer " + access_token;
	curl_slist * header = curl_slist_append(NULL, auth_header.c_str());

	GET("https://discord.com/api/v9/users/@me/guilds", header, NULL, NULL);

	picojson::value json_doc;
	picojson::parse(json_doc, input);

	if (json_doc.is<picojson::array>()) {
		const picojson::array & json_arr = json_doc.get<picojson::array>();
		for (const picojson::value & guild_val : json_arr) {
			guilds.push_back(guild_val);
		}
	}

	std::sort(guilds.begin(), guilds.end(), [](const Guild & a, const Guild & b){ return a.name < b.name; });

	return guilds;
}

std::vector<Channel> get_guild_channels(const int64_t guild_id, const std::string & bot_token) {

    std::vector<Channel> channels;

	std::string auth_header = "Authorization: Bot " + bot_token;
	curl_slist * header = curl_slist_append(NULL, auth_header.c_str());

	std::stringstream url;
	url << "https://discord.com/api/v9/guilds/";
	url << guild_id;
	url << "/channels";

	GET(url.str().c_str(), header, NULL, NULL);

	picojson::value json_doc;
	picojson::parse(json_doc, input);

	if (json_doc.is<picojson::array>()) {
		const picojson::array & json_arr = json_doc.get<picojson::array>();
		for (const picojson::value & channel_val : json_arr) {
			channels.push_back(channel_val);
		}
	}

	std::sort(channels.begin(), channels.end(), [](const Channel & a, const Channel & b){ return a.name < b.name; });

	return channels;
}

bool bot_in_guild(const int64_t guild_id) {
	std::string auth_header = "Authorization: Bot " + BOT_TOKEN;
	curl_slist * header = curl_slist_append(NULL, auth_header.c_str());

	std::string url = (std::stringstream() << DISCORD_API "guilds/" << guild_id << "/members/" CLIENT_ID).str();
	CURLcode code = GET(url, header, NULL, NULL);
	curl_slist_free_all(header);

    return code >= 200 && code < 300;
}