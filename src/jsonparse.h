#ifndef jsonparse_h
#define jsonparse_h

#include "pipeline.h"

#include <glib.h>

typedef enum {

} RequestType

typedef struct {
    gint duration;
    gint position;
} ClientRequest;

ClientRequest *jsonparse_play_request(Client *client);

#endif /* jsonparse_h */
