#ifndef _H_CCSCRIPTS
#define _H_CCSCRIPTS

#include "civetweb.h"

int ccscripts_login(mg_connection *conn, void *cbdata);

int ccscripts_callback(struct mg_connection * conn, void * cbdata);

int ccscripts_create_login(struct mg_connection * conn, void * cbdata);

#endif