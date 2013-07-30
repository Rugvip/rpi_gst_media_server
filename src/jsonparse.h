#ifndef jsonparse_h
#define jsonparse_h

#include "common.h"
#include <json-glib/json-glib.h>

Request *jsonparse_read_request(Client *client, JsonParser *parser);

#endif /* jsonparse_h */
