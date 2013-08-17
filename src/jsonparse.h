#ifndef jsonparse_h
#define jsonparse_h

#include "common.h"
#include <json-glib/json-glib.h>

Request *jsonparse_read_request(Player *, JsonParser *);

#endif /* jsonparse_h */
