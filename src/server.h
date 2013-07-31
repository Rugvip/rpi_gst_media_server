#ifndef server_h
#define server_h

#include "common.h"

gboolean server_print_connections(Server *server);
Server *server_new();
void server_init(Server *server);
void server_start(Server *server, gint port);

#endif /* server_h */
