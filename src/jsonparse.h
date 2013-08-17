#ifndef jsonparse_h
#define jsonparse_h

#include "common.h"
#include <json-glib/json-glib.h>

Input *jsonparse_read_input(Player *, JsonParser *);

#endif /* jsonparse_h */
