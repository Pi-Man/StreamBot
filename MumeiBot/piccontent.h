#ifndef _H_PICCONTENT
#define _H_PICCONTENT

int pic_home_callback(struct mg_connection * conn, void * cbdata);

int pic_find_server_callback(struct mg_connection * conn, void * cbdata);

int pic_server_callback(struct mg_connection * conn, void * cbdata);

int pic_dm_callback(struct mg_connection * conn, void * cbdata);

int pic_server_create_callback(struct mg_connection * conn, void * cbdata);

int pic_server_join_callback(struct mg_connection * conn, void * cbdata);

int pic_server_manage_callback(struct mg_connection * conn, void * cbdata);

int pic_server_add_channel_callback(struct mg_connection * conn, void * cbdata);

int pic_server_remove_channel_callback(struct mg_connection * conn, void * cbdata);

int pic_post_callback(struct mg_connection * conn, void * cbdata);

#endif