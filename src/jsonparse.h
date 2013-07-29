#ifndef jsonparse_h
#define jsonparse_h

#include "jsonio.h"
#include "pipeline.h"

#include <json-glib/json-glib.h>

Request *jsonparse_read_request(Client *client, JsonParser *parser);

#endif /* jsonparse_h */
