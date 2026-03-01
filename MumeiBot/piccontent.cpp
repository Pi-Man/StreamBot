#include <vector>

#include <pqxx/pqxx>

#include "civetweb.h"

#include "piclogin.h"
#include "httpheaders.h"
#include "util.h"
#include "picserver.h"
#include "piccontent.h"

int pic_home_callback(struct mg_connection * conn, void * cbdata) {

    const mg_request_info * info = mg_get_request_info(conn);

    HTTPHeaders request_headers(info->http_headers, info->num_headers);

    std::string user = pic_authenticate(request_headers.cookies);

    if (user.empty()) {
        mg_send_http_error(conn, 401, "Unauthorized");
        return 401;
    }

    HTTPHeaders response_headers;
    response_headers["Content-Type"] = "text/html";
    response_headers["Connection"] = "close";

    mg_printf(conn, "%s", "HTTP/1.1 200 OK\r\n");

    mg_printf(conn, "%s", std::string(response_headers).c_str());

    std::string friends = format(load_file(WEB_ROOT "/PiC/home/friend_entry.html"), {"user", "user", "Some User", "doing stuff"});
    std::string joined;
    std::string owned;

    for (const PiC_Server & server : get_user_joined_servers(user)) {
        joined += format(load_file(WEB_ROOT "/PiC/home/server_entry.html"), {server.name, server.name, server.name});
    }

    for (const PiC_Server & server : get_user_owned_servers(user)) {
        owned += format(load_file(WEB_ROOT "/PiC/home/server_entry.html"), {server.name, server.name, server.name});
    }

    mg_printf(conn, load_file(WEB_ROOT "/PiC/home/index.html").c_str(), friends.c_str(), joined.c_str(), owned.c_str());

    return 200;
}

int pic_find_server_callback(struct mg_connection * conn, void * cbdata) {

    const mg_request_info * info = mg_get_request_info(conn);

    HTTPHeaders request_headers(info->http_headers, info->num_headers);

    std::string user = pic_authenticate(request_headers.cookies);

    if (user.empty()) {
        mg_send_http_error(conn, 401, "Unauthorized");
        return 401;
    }

    HTTPHeaders response_headers;
    response_headers["Content-Type"] = "text/html";
    response_headers["Connection"] = "close";

    mg_printf(conn, "%s", "HTTP/1.1 200 OK\r\n");

    mg_printf(conn, "%s", std::string(response_headers).c_str());

    std::string servers;

    for (const PiC_Server & server : get_all_servers(user)) {
        servers += format(load_file(WEB_ROOT "/PiC/find_server/server_entry.html"), {server.name, server.name, server.name});
    }

    mg_printf(conn, load_file(WEB_ROOT "/PiC/find_server/index.html").c_str(), servers.c_str());

    return 200;
}

int pic_server_callback(struct mg_connection * conn, void * cbdata) {

    const mg_request_info * info = mg_get_request_info(conn);

    HTTPHeaders request_headers(info->http_headers, info->num_headers);
    HTMLForm request_form(info->query_string);

    std::string user = pic_authenticate(request_headers.cookies);

    if (user.empty()) {
        mg_send_http_error(conn, 401, "Unauthorized");
        return 401;
    }

    std::string url = info->request_uri;

    std::string server_name = url.substr(strlen("/PiC/Server/"));

    if (!pic_server_exists(server_name)) {
        return 0;
    }

    HTTPHeaders response_headers;
    response_headers["Content-Type"] = "text/html";
    response_headers["Connection"] = "close";

    mg_printf(conn, "%s", "HTTP/1.1 200 OK\r\n");

    mg_printf(conn, "%s", std::string(response_headers).c_str());

    std::string joined;
    std::string channels;
    std::string chat;
    std::string channel = request_form.has("c") ? request_form["c"] : std::string{};
    if (!pic_server_has_channel(server_name, channel)) channel.clear();

    if (!channel.empty()) {
        for (const std::pair<std::string, std::string> & message_entry : get_chat(server_name, channel)) {
            chat += format(load_file(WEB_ROOT "/PiC/server/chat_entry.html"), {message_entry.first, message_entry.second});
        }
    }

    for (const PiC_Server & server : get_user_joined_servers(user)) {
        joined += format(load_file(WEB_ROOT "/PiC/server/server_entry.html"), {server.name, server.name, server.name});
    }

    for (const std::string & channel : get_server_channels(server_name)) {
        channels += format(load_file(WEB_ROOT "/PiC/server/channel_entry.html"), {server_name, channel, channel});
    }

    mg_printf(conn, load_file(WEB_ROOT "/PiC/server/index.html").c_str(), 
    server_name.c_str(), 
    server_name.c_str(), 
    joined.c_str(), 
    pic_is_server_owner(server_name, user) ? format(load_file(WEB_ROOT "/PiC/server/add_channel.html"), {server_name} ).c_str() : "", 
    channels.c_str(), 
    server_name.c_str(), 
    channel.c_str(), 
    chat.c_str());

    return 200;

}

int pic_dm_callback(struct mg_connection * conn, void * cbdata);

int pic_server_create_callback(struct mg_connection * conn, void * cbdata) {

    const mg_request_info * info = mg_get_request_info(conn);

    HTTPHeaders request_headers(info->http_headers, info->num_headers);

    HTMLForm request_form{get_body(conn)};

    std::string user = pic_authenticate(request_headers.cookies);

    if (user.empty()) {
        mg_send_http_error(conn, 401, "Unauthorized");
        return 401;
    }

    if (!request_form.has("name")) {
        mg_send_http_error(conn, 400, "Bad Request");
    }

    pic_create_server(request_form["name"], "", user);

    mg_send_http_redirect(conn, "/PiC/home", 303);

    return 303;

}

int pic_server_join_callback(struct mg_connection * conn, void * cbdata) {

    const mg_request_info * info = mg_get_request_info(conn);

    HTTPHeaders request_headers(info->http_headers, info->num_headers);

    std::string user = pic_authenticate(request_headers.cookies);

    if (user.empty()) {
        mg_send_http_error(conn, 401, "Unauthorized");
        return 401;
    }

    std::string url = info->request_uri;

    std::string server_name = url.substr(strlen("/PiC/api/join/"));

    if (!pic_server_exists(server_name)) {
        mg_send_http_error(conn, 400, "Bad Request");
        return 400;
    }

    pic_join_server(server_name, user);

    mg_send_http_redirect(conn, "/PiC/home", 303);

    return 303;
}

int pic_server_manage_callback(mg_connection *conn, void *cbdata) {

    const mg_request_info * info = mg_get_request_info(conn);

    HTTPHeaders request_headers(info->http_headers, info->num_headers);

    std::string user = pic_authenticate(request_headers.cookies);

    if (user.empty()) {
        mg_send_http_error(conn, 401, "Unauthorized");
        return 401;
    }

    std::string url = info->request_uri;

    std::string server_name = url.substr(strlen("/PiC/Server/"));
    server_name = server_name.substr(0, server_name.find('/'));

    if (!pic_server_exists(server_name)) {
        return 0;
    }

    if (!pic_is_server_owner(server_name, user)) {
        mg_send_http_error(conn, 401, "Unauthorized");
        return 401;
    }

    HTTPHeaders response_headers;
    response_headers["Content-Type"] = "text/html";
    response_headers["Connection"] = "close";

    mg_printf(conn, "%s", "HTTP/1.1 200 OK\r\n");

    mg_printf(conn, "%s", std::string(response_headers).c_str());

    std::string channels;

    for (std::string & channel : get_server_channels(server_name)) {
        channels += format(load_file(WEB_ROOT "/PiC/server/manage/channel_entry.html"), {channel, server_name});
    }

    mg_printf(conn, load_file(WEB_ROOT "/PiC/server/manage/index.html").c_str(), 
    server_name.c_str(), 
    server_name.c_str(), 
    server_name.c_str(), 
    server_name.c_str(), 
    channels.c_str());

    return 200;

}

int pic_server_add_channel_callback(mg_connection *conn, void *cbdata) {

    const mg_request_info * info = mg_get_request_info(conn);

    HTTPHeaders request_headers(info->http_headers, info->num_headers);

    HTMLForm request_form(get_body(conn));

    std::string user = pic_authenticate(request_headers.cookies);

    if (user.empty()) {
        mg_send_http_error(conn, 401, "Unauthorized");
        return 401;
    }

    std::string url = info->request_uri;

    std::string server_name = url.substr(strlen("/PiC/api/server/"));
    server_name = server_name.substr(0, server_name.find('/'));

    if (!pic_server_exists(server_name) || !request_form.has("channel")) {
        return 0;
    }

    pic_create_server_channel(server_name, request_form["channel"]);

    mg_send_http_redirect(conn, ("/PiC/Server/" + server_name + "/Manage").c_str(), 303);
    return 303;
}

int pic_server_remove_channel_callback(mg_connection *conn, void *cbdata) {

    const mg_request_info * info = mg_get_request_info(conn);

    HTTPHeaders request_headers(info->http_headers, info->num_headers);

    HTMLForm request_form(get_body(conn));

    std::string user = pic_authenticate(request_headers.cookies);

    if (user.empty()) {
        mg_send_http_error(conn, 401, "Unauthorized");
        return 401;
    }

    std::string url = info->request_uri;

    std::string server_name = url.substr(strlen("/PiC/api/server/"));
    server_name = server_name.substr(0, server_name.find('/'));

    if (!pic_server_exists(server_name) || !request_form.has("channel")) {
        return 0;
    }

    pic_delete_server_channel(server_name, request_form["channel"]);

    mg_send_http_redirect(conn, ("/PiC/Server/" + server_name + "/Manage").c_str(), 303);
    return 303;
}

int pic_post_callback(mg_connection *conn, void *cbdata) {

    const mg_request_info * info = mg_get_request_info(conn);

    HTTPHeaders request_headers(info->http_headers, info->num_headers);

    HTMLForm request_form(get_body(conn));

    std::string user = pic_authenticate(request_headers.cookies);

    if (user.empty()) {
        mg_send_http_error(conn, 401, "Unauthorized");
        return 401;
    }

    if (!request_form.has("server") || !request_form.has("channel")) {
        mg_send_http_error(conn, 400, "Bad Request");
        return 400;
    }

    if (!request_form.has("refresh")) {
        if (!request_form.has("message")) {
            mg_send_http_error(conn, 400, "Bad Request");
            return 400;
        }

        pic_post_message(request_form["server"], request_form["channel"], user, request_form["message"]);
    }

    mg_send_http_redirect(conn, ("/PiC/Server/" + request_form["server"] + "?c=" + request_form["channel"]).c_str(), 303);
    return 303;
}
