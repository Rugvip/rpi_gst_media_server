#ifndef jsonapi_h
#define jsonapi_h

#include "pipeline.h"

#include <glib.h>

typedef enum {
    REQUEST_PLAY,
    REQUEST_PAUSE,
    REQUEST_NEXT,
    REQUEST_SEEK,
    REQUEST_INFO,
    REQUEST_,
} RequestType

typedef struct {
    RequestType type;
    Client *client;
} Request;

typedef struct {
    Request request;

} PlayRequest;

typedef struct {
    Request request;
} PauseRequest;

typedef struct {
    Request request;
} NextRequest;

typedef struct {
    Request request;
} SeekRequest;

typedef void (*RequestHandler)(Request *);

void jsonapi_read_request(Client *);
void jsonapi_set_request_handler(RequestType, RequestHandler);

#endif /* jsonapi_h */
