#ifndef _H_REGISTER
#define _H_REGISTER

#include "civetweb.h"

int login_callback(struct mg_connection * conn, void * cbdata);

int logout_callback(struct mg_connection * conn, void * cbdata);

int register_callback(struct mg_connection * conn, void * cbdata);

int oauth_callback(struct mg_connection * conn, void * cbdata);

#endif