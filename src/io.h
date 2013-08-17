#ifndef server_h
#define server_h

#include "common.h"

Server *server_new();
void server_init(Server *server);
void server_start(Server *server, gint port);

#endif /* server_h */
