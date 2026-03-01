#ifndef _H_PICLOGIN
#define _H_PICLOGIN

#include "string"
#include "httpheaders.h"

std::string pic_authenticate(const HTTPCookies & cookies);

int pic_login_callback(struct mg_connection * conn, void * cbdata);

int pic_register_callback(struct mg_connection *conn, void * cbdata);

#endif