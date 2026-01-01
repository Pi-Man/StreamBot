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
        std::sscanf(str.c_str(), "%d.%d.%d.%d", &maj, &min, &patch, &build);
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

    operator std::string() {
        std::stringstream builder;
        builder << maj << '.' << min << '.' << patch << '.' << build;
        return builder.str();
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
                pqxx::result table = work.exec_params("SELECT id FROM ccscriptsuser WHERE user_name=$1 AND hash=$2", user, hash);
                if (table.size() == 1) {
                    work.commit();
                    return 1;
                }
            }
            catch (pqxx::data_exception & e) {
                mg_send_http_error(conn, 401, "Invalid Credentials");
                return 0;
            }
        }
    }
    mg_send_http_error(conn, 401, "No Credentials Given");
    return 0;
}

int ccscripts_callback(mg_connection *conn, void *cbdata) {

    const mg_request_info * info = mg_get_request_info(conn);

    std::string method = info->request_method;
    if (method == "GET") return 0;

    HTMLForm query(info->query_string);
    HTTPHeaders request_headers(info->http_headers, info->num_headers);
    std::string body = get_body(conn);

    std::string path = info->local_uri;
    std::size_t index = path.rfind('/');
    std::string filename = path.substr(index + 1);

    if (query.has("version")) {
        std::string user, hash;
        std::tie(user, hash) = get_user_hash_basicauth(request_headers);

        if (!(user.empty() || hash.empty())) {

            pqxx::connection pqconn(CONN_STR);
            pqxx::work work(pqconn);
            pqxx::result table = work.exec_params("SELECT id FROM ccscriptsuser WHERE user_name=$1 AND hash=$2", user, hash);
            size_t user_id = table[0][0].as<size_t>();
            work.exec("CREATE TABLE IF NOT EXISTS ccscript (id SERIAL PRIMARY KEY, user_id INT, version TEXT, filename VARCHAR(128))");
            table = work.exec_params("SELECT user_id, version FROM ccscript WHERE filename=$1", filename);
            CCScriptVersion new_version(query["version"]);
            if (table.size() == 1) {
                if (table[0][0].as<size_t>() != user_id) {
                    mg_send_http_error(conn, 403, "Unauthorized");
                    return 403;
                }
                CCScriptVersion old_version(table[0][1].as<std::string>());
                if (new_version <= old_version) {
                    mg_send_http_error(conn, 409, "Version is Not Newer");
                    return 409;
                }
                work.exec_params("UPDATE ccscript SET version=$1 WHERE filename=$2", (std::string) new_version, filename);
            }
            else {
                work.exec_params("INSERT INTO ccscript (user_id, version, filename) VALUES ($1, $2, $3)", user_id, (std::string) new_version, filename);
            }
            work.commit();
            std::string filepath = (WEB_ROOT "cc-scripts/") + filename;
            FILE * file = fopen(filepath.c_str(), "w");
            if (file) {
                fwrite(body.c_str(), 1, body.length(), file);
                fclose(file);
                mg_send_http_error(conn, 204, "");
                return 204;
            }
            else {
                mg_send_http_error(conn, 500, "File Could Not Be Created");
                return 500;
            }
        }
        else {
            mg_send_http_error(conn, 401, "No Credentials Given");
            return 401;
        }
    }
    mg_send_http_error(conn, 400, "No Version Given");

    return 0;
}

int ccscripts_create_login(mg_connection *conn, void *cbdata) {

    HTMLForm formdata(get_body(conn));

    if (formdata.has("user") && formdata.has("pass")) {
        std::string user = formdata["user"];
        std::string pass = formdata["pass"];
        std::error_code err;
        std::string hash = bstos(hasher.sign(pass, err));
        if (err) {
            mg_send_http_error(conn, 500, "Server Error");
            return 500;
        }
        pqxx::connection pqconn(CONN_STR);
        pqxx::work work(pqconn);
        work.exec("CREATE TABLE IF NOT EXISTS ccscriptsuser (id SERIAL PRIMARY KEY, user_name VARCHAR(64), hash TEXT)");
        pqxx::result table = work.exec_params("SELECT id FROM ccscriptsuser WHERE user_name=$1", user);
        if (table.size() != 0) {
            mg_send_http_redirect(conn, "/cc-scripts/bad_username", 303);
            return 500;
        }
        work.exec_params("INSERT INTO ccscriptsuser (user_name, hash) VALUES ($1, $2)", user, hash);
        work.commit();
        mg_send_http_redirect(conn, "/cc-scripts/success", 303);
        return 204;
    }

    return 0;
}
