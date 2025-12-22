#ifndef _H_DISCORD
#define _H_DISCORD

#include <string>
#include <vector>

#include "guild.h"
#include "channel.h"

std::string get_user_name(const std::string & access_token);

std::vector<Guild> get_user_guilds(const std::string & access_token);

std::vector<Channel> get_guild_channels(const int64_t guild_id, const std::string & bot_token);

bool bot_in_guild(const int64_t guild_id);

#endif