#ifndef jsonparse_h
#define jsonparse_h

#include "common.h"
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

Input *jsonparse_read_input(Player *, JsonParser *);

G_END_DECLS

#endif /* jsonparse_h */
