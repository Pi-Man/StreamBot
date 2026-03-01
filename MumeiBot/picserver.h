#ifndef _H_PICSERVER
#define _H_PICSERVER

#include <string>
#include <vector>

struct PiC_Server {
    std::string name;
    std::vector<std::string> channels;
};

bool pic_server_exists(std::string name);

bool pic_is_server_owner(std::string name, std::string user);

void pic_create_server(std::string name, std::string pfp, std::string owner);

void pic_create_server_channel(std::string server, std::string channel);

void pic_delete_server_channel(std::string server, std::string channel);

void pic_join_server(std::string name, std::string user);

bool pic_server_has_channel(std::string server, std::string channel);

void pic_post_message(std::string server, std::string channel, std::string message);

std::vector<PiC_Server> get_all_servers(std::string user);

std::vector<PiC_Server> get_user_joined_servers(std::string user);

std::vector<PiC_Server> get_user_owned_servers(std::string user);

std::vector<std::string> get_server_channels(std::string server);

std::vector<std::string> get_chat(std::string server, std::string channel);

#endif