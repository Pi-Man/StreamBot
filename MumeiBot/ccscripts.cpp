#include "ccscripts.h"

#include <string>
#include <regex>
#include <tuple>

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/base.h>
#include <pqxx/pqxx>

#include "httpheaders.h"
#include "htmlform.h"
#include "util.h"

struct CCScriptVersion {
    int maj = 0, min = 0, patch = 0, build = 0;

    CCScriptVersion(const std::string & str) {
        size_t idx = 0;
        if (!std::isdigit(str[idx])) return;
        maj = std::stoi(str, &idx);
        if (str[idx] != '.') return;
        idx++;
        if (!std::isdigit(str[idx])) return;
        min = std::stoi(str, &idx);
        if (str[idx] != '.') return;
        idx++;
        if (!std::isdigit(str[idx])) return;
        patch = std::stoi(str, &idx);
        if (str[idx] != '.') return;
        idx++;
        if (!std::isdigit(str[idx])) return;
        build = std::stoi(str, &idx);
    }

    bool operator>(const CCScriptVersion & other) {
        if (maj < other.maj) return false;
        if (maj > other.maj) return true;
        if (min < other.min) return false;
        if (min > other.min) return true;
        if (patch < other.patch) return false;
        if (patch > other.patch) return true;
        if (build < other.build) return false;
        if (build > other.build) return true;
        return false;
    }
    bool operator<(const CCScriptVersion & other) {
        if (maj > other.maj) return false;
        if (maj < other.maj) return true;
        if (min > other.min) return false;
        if (min < other.min) return true;
        if (patch > other.patch) return false;
        if (patch < other.patch) return true;
        if (build > other.build) return false;
        if (build < other.build) return true;
        return false;
    }
    bool operator>=(const CCScriptVersion & other) {
        if (maj < other.maj) return false;
        if (maj > other.maj) return true;
        if (min < other.min) return false;
        if (min > other.min) return true;
        if (patch < other.patch) return false;
        if (patch > other.patch) return true;
        if (build < other.build) return false;
        if (build > other.build) return true;
        return true;
    }
    bool operator<=(const CCScriptVersion & other) {
        if (maj > other.maj) return false;
        if (maj < other.maj) return true;
        if (min > other.min) return false;
        if (min < other.min) return true;
        if (patch > other.patch) return false;
        if (patch < other.patch) return true;
        if (build > other.build) return false;
        if (build < other.build) return true;
        return true;
    }
};



int ccscripts_login(mg_connection *conn, void *cbdata) {

    const mg_request_info * info = mg_get_request_info(conn);

    std::string method = info->request_method;
    if (method == "GET") return 1;
    if (method == "POST") {
        HTTPHeaders request_headers(info->http_headers, info->num_headers);
        std::string user, hash;
        std::tie(user, hash) = get_user_hash_basicauth(request_headers);
        if (!(user.empty() || hash.empty())) {
            try {
                pqxx::connection pqconn(CONN_STR);
                pqxx::read_transaction work(pqconn);
                pqxx::result table = work.exec_params("SELECT id FROM ccscriptuser WHERE user=$1 AND hash=$2", user, hash);
                if (table.size() == 1) {
                    work.commit();
                    return 1;
                }
            }
            catch (pqxx::data_exception & e) {
                return 0;
            }
        }
    }
    return 0;
}

int ccscripts_callback(mg_connection *conn, void *cbdata) {

    const mg_request_info * info = mg_get_request_info(conn);

    HTMLForm query(info->query_string);
    HTTPHeaders request_headers(info->http_headers, info->num_headers);
    std::string body = get_body(conn);

    if (
        query.has("filename") &&
        query.has("version")
    ) {
        std::string user, hash;
        std::tie(user, hash) = get_user_hash_basicauth(request_headers);

        pqxx::connection pqconn(CONN_STR);
        pqxx::work work(pqconn);
        pqxx::read_transaction read_work(pqconn);
        pqxx::result table = read_work.exec_params("SELECT id FROM ccscriptsuser WHERE user=$1 AND hash=$1", user, hash);
        size_t user_id = table[0][0].as<size_t>();
        table = work.exec_params("SELECT user, version FROM ccscript WHERE filename=$1", query["filename"]);
        if (table.size() == 1) {
            if (table[0][0].as<size_t>() != user_id) {
                mg_send_http_error(conn, 403, "");
                return 403;
            }
            CCScriptVersion new_version(query["version"]);
            CCScriptVersion old_version(table[0][1].as<std::string>());
            if (new_version <= old_version) {
                mg_send_http_error(conn, 409, "");
                return 409;
            }
        }
        std::string filename = (WEB_ROOT "cc-scripts/") + query["filename"];
        FILE * file = fopen(filename.c_str(), "w");
        if (file) {
            fwrite(body.c_str(), 1, body.length(), file);
            mg_send_http_error(conn, 204, "");
        }
        else {
            mg_send_http_error(conn, 500, "");
            return 500;
        }
    }

    return 0;
}