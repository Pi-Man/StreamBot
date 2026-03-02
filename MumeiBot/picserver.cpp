#include "picserver.h"

#include <pqxx/pqxx>

#include "civetweb.h"

#include "util.h"

static void build_server_table(pqxx::work & work) {
	work.exec("CREATE TABLE IF NOT EXISTS pic_server(id SERIAL PRIMARY KEY, name VARCHAR(64), owner VARCHAR(64), pfp VARCHAR(256))");
}
static void build_server_user_table(pqxx::work & work) {
	work.exec("CREATE TABLE IF NOT EXISTS pic_server_user(server_name VARCHAR(64), user_name VARCHAR(64), PRIMARY KEY (server_name, user_name))");
}
static void build_channel_table(pqxx::work & work) {
	work.exec("CREATE TABLE IF NOT EXISTS pic_channel(id SERIAL PRIMARY KEY, name VARCHAR(64), server_name VARCHAR(64))");
}
static void build_message_table(pqxx::work & work) {
	work.exec("CREATE TABLE IF NOT EXISTS pic_message(id SERIAL PRIMARY KEY, server_name VARCHAR(64), channel_name VARCHAR(64), user_name VARCHAR(64), message TEXT)");
}
static void join_server(pqxx::work & work, std::string server, std::string user) {
    work.exec_params("INSERT INTO pic_server_user VALUES ($1, $2)", server, user);
}

bool pic_server_exists(std::string name) {

    pqxx::connection pqconn(CONN_STR);
    pqxx::work work(pqconn);

    build_server_table(work);

    pqxx::result table = work.exec_params("SELECT name FROM pic_server WHERE name=$1", name);

    return !table.empty();
}

bool pic_is_server_owner(std::string name, std::string user) {

    pqxx::connection pqconn(CONN_STR);
    pqxx::work work(pqconn);

    build_server_table(work);

    pqxx::result table = work.exec_params("SELECT owner FROM pic_server WHERE name=$1 AND owner=$2", name, user);

    return !table.empty();
}

void pic_create_server(std::string name, std::string pfp, std::string owner) {
    pqxx::connection pqconn(CONN_STR);
    pqxx::work work(pqconn);

	build_server_table(work);
    build_server_user_table(work);

    work.exec_params("INSERT INTO pic_server(name, owner) VALUES ($1, $2)", name, owner);

    join_server(work, name, owner);

    work.commit();
}

void pic_create_server_channel(std::string server, std::string channel) {

    pqxx::connection pqconn(CONN_STR);
    pqxx::work work(pqconn);

    build_channel_table(work);

    work.exec_params("INSERT INTO pic_channel(name, server_name) VALUES ($1, $2)", channel, server);

    work.commit();

}

void pic_delete_server_channel(std::string server, std::string channel) {

    pqxx::connection pqconn(CONN_STR);
    pqxx::work work(pqconn);

    build_channel_table(work);

    work.exec_params("DELETE FROM pic_channel WHERE name=$1 AND server_name=$2", channel, server);

    work.commit();
    
}

void pic_join_server(std::string name, std::string user) {

    pqxx::connection pqconn(CONN_STR);
    pqxx::work work(pqconn);

    build_server_table(work);
    build_server_user_table(work);

    join_server(work, name, user);

    work.commit();

}

bool pic_server_has_channel(std::string server, std::string channel) {

    pqxx::connection pqconn(CONN_STR);
    pqxx::work work(pqconn);

    build_channel_table(work);

    pqxx::result table = work.exec_params("SELECT name FROM pic_channel WHERE name=$1 AND server_name=$2", channel, server);

    return !table.empty();
}

void pic_post_message(std::string server, std::string channel, std::string user, std::string message) {

    pqxx::connection pqconn(CONN_STR);
    pqxx::work work(pqconn);

    build_message_table(work);

    work.exec_params("INSERT INTO pic_message(server_name, channel_name, user_name, message) VALUES ($1, $2, $3, $4)", server, channel, user, message);

    work.commit();

}

std::vector<PiC_Server> get_all_servers(std::string user) {
    
    pqxx::connection pqconn(CONN_STR);
    pqxx::work work(pqconn);

	build_server_table(work);
    build_server_user_table(work);

    pqxx::result table = work.exec("SELECT name FROM pic_server");

    std::vector<PiC_Server> servers;

    for (const pqxx::row & row : table) {
        servers.push_back({
            row[0].as<std::string>(),
            {}
        });
    }

    return servers;
}

std::vector<PiC_Server> get_user_joined_servers(std::string user)
{

    pqxx::connection pqconn(CONN_STR);
    pqxx::work work(pqconn);

	build_server_table(work);
    build_server_user_table(work);

    pqxx::result table = work.exec_params("SELECT pic_server.name FROM pic_server_user JOIN pic_server ON pic_server_user.server_name=pic_server.name WHERE pic_server_user.user_name=$1", user);

    std::vector<PiC_Server> servers;

    for (const pqxx::row & row : table) {
        servers.push_back({
            row[0].as<std::string>(),
            {}
        });
    }

    return servers;
}

std::vector<PiC_Server> get_user_owned_servers(std::string user) {

    pqxx::connection pqconn(CONN_STR);
    pqxx::work work(pqconn);

	build_server_table(work);

    pqxx::result table = work.exec_params("SELECT name FROM pic_server WHERE owner=$1", user);

    std::vector<PiC_Server> servers;

    for (const pqxx::row & row : table) {
        servers.push_back({
            row[0].as<std::string>(),
            {}
        });
    }

    return servers;

}

std::vector<std::string> get_server_channels(std::string server) {

    pqxx::connection pqconn(CONN_STR);
    pqxx::work work(pqconn);

    build_channel_table(work);

    pqxx::result table = work.exec_params("SELECT name FROM pic_channel WHERE server_name=$1", server);

    std::vector<std::string> channels;
        for (const pqxx::row & row : table) {
        channels.push_back(
            row[0].as<std::string>()
        );
    }

    return channels;
}

std::string & html_sanitize(std::string & str) {

    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '<') {
            str.replace(i, 1, "&lt;");
        }
        else if (str[i] == '>') {
            str.replace(i, 1, "&gt;");
        }
        else if (str[i] == '&') {
            str.replace(i, 1, "$amp;");
        }
    }

    return str;

}

std::vector<std::pair<std::string, std::string>> get_chat(std::string server, std::string channel) {

    pqxx::connection pqconn(CONN_STR);
    pqxx::work work(pqconn);

    build_message_table(work);

    pqxx::result table = work.exec_params("SELECT user_name, message FROM pic_message WHERE server_name=$1 AND channel_name=$2", server, channel);

    std::vector<std::pair<std::string, std::string>> chat;
    for (const pqxx::row & row : table) {
        std::string user = row[0].as<std::string>();
        std::string message = row[1].as<std::string>();

        chat.push_back({
            html_sanitize(user),
            html_sanitize(message)
        });
    }

    return chat;
}
