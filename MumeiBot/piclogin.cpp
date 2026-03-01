#include "piclogin.h"

#include <jwt-cpp/jwt.h>
#include <pqxx/pqxx>

#include "civetweb.h"

#include "htmlform.h"
#include "util.h"

std::string pic_authenticate(const HTTPCookies & cookies) {
	if (cookies.has("JWT")) {

		jwt::decoded_jwt jwt = jwt::decode(cookies["JWT"]);
		jwt::claim user = jwt.get_payload_claim("User");
		jwt::verifier verifier = jwt::verify()
			.with_type("JWT")
			.with_issuer("PiC")
			.with_claim("User", user)
			.allow_algorithm(jwt::algorithm::hs256(load_file("pic_secret.txt")));
		std::error_code error;
		verifier.verify(jwt, error);

		if (error) return "";
		
        return user.as_string();
	}
	return "";
}

int pic_login_callback(mg_connection *conn, void *cbdata) {

    std::string body = get_body(conn);

    HTMLForm request_form(body);

    std::string token;

    if (request_form.has("anon")) {

        token = jwt::create()
            .set_type("JWT")
            .set_issuer("PiC")
            .set_issued_now()
            .set_payload_claim("User", jwt::claim(std::string("anon")))
            .sign(jwt::algorithm::hs256(load_file("pic_secret.txt")));

    }
    else {
        std::string user;
        std::string hash;
        std::tie(user, hash) = get_user_hash_form(request_form, "username", "password");

        pqxx::connection pqconn(CONN_STR);
        pqxx::read_transaction work(pqconn);
        pqxx::result table = work.exec_params("SELECT * FROM pic_user WHERE id=$1 AND hash=$2", user, hash);
        work.commit();
        if (table.size() != 1) {
            mg_send_http_error(conn, 401, "Unauthorized");
            return 401;
        }

        token = jwt::create()
            .set_type("JWT")
            .set_issuer("PiC")
            .set_issued_now()
            .set_payload_claim("User", jwt::claim(user))
            .sign(jwt::algorithm::hs256(load_file("pic_secret.txt")));

    }

    mg_printf(conn,
        "HTTP/1.1 303 See Other\r\n"
        "Location: /PiC/home\r\n"
        "Content-Length: 0\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
#ifdef _DEBUG
        "Set-Cookie: JWT=%s; Path=/PiC/; SameSite=Lax; Max-Age=%llu; HttpOnly\r\n"
#else
        "Set-Cookie: JWT=%s; Path=/PiC/; SameSite=Lax; Max-Age=%llu; Secure; HttpOnly\r\n"
#endif
        "\r\n"
        ,
        token.c_str(),
        24llu * 3600llu);

    return 303;
}

int pic_register_callback(mg_connection *conn, void *cbdata) {

    std::string body = get_body(conn);

    HTMLForm request_form(body);

    std::string user;
    std::string hash;
    std::tie(user, hash) = get_user_hash_form(request_form, "username", "password");

    if (user.empty() || user.compare(0, 4, "anon") == 0) {
        mg_send_http_error(conn, 406, "Not Acceptable");
        return 406;
    }

    pqxx::connection pqconn(CONN_STR);
    pqxx::work work(pqconn);

    work.exec("CREATE TABLE IF NOT EXISTS pic_user (id VARCHAR(64) PRIMARY KEY, hash TEXT)");
    pqxx::result table = work.exec_params("SELECT * FROM pic_user WHERE id=$1", user);

    if (!table.empty()) {
        mg_send_http_error(conn, 406, "Not Acceptable");
        return 406;
    }

    work.exec_params("INSERT INTO pic_user (id, hash) VALUES ($1, $2)", user, hash);

    work.commit();

    mg_send_http_redirect(conn, "/PiC/login", 303);
    return 303;

}