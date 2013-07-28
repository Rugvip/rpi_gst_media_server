#ifndef jsonapi_h
#define jsonapi_h

#include "pipeline.h"
#include "jsongen.h"

#include <glib.h>

typedef enum {
    REQUEST_PLAY,
    REQUEST_PAUSE,
    REQUEST_NEXT,
    REQUEST_SEEK,
    REQUEST_INFO,
    NUM_REQUEST_TYPES,
} RequestType

typedef struct {
    RequestType type;
    Client *client;
} Request;

/* Song to play starting at time */
typedef struct {
    Request request;
    Song song;
    gint time;
} RequestPlay;

/* Pause the playing song and seek to time */
typedef struct {
    Request request;
    gint time;
} RequestPause;

/* The next song to be played */
typedef struct {
    Request request;
    Song song;
    gint time;
} RequestNext;

/* Seek to a specific time */
typedef struct {
    Request request;
    gint time;
} RequestSeek;

typedef void (*RequestHandler)(Request *);

void jsonapi_read_request(Client *);
void jsonapi_set_request_handler(RequestType, RequestHandler);

void jsongen_send_packet(Client *, JsonPacket *);
void jsongen_broadcast_packet(Server *, JsonPacket *);

#endif /* jsonapi_h */
