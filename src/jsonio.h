#ifndef jsonapi_h
#define jsonapi_h

#include "pipeline.h"
#include "jsongen.h"

#include <glib.h>

typedef enum {
    REQUEST_INFO,
    REQUEST_PLAY,
    REQUEST_PAUSE,
    REQUEST_NEXT,
    REQUEST_SEEK,
    REQUEST_VOLUME,
    REQUEST_EQ,
    NUM_REQUEST_TYPES,
} RequestType

typedef struct {
    RequestType type;
    Client *client;
} Request;

/* Request to send information */
typedef struct {
    Request request;
} RequestInfo;

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

/* Set volume */
typedef struct {
    Request request;
    gint volume;
} RequestVolume;

/* Set equalizer settings */
typedef struct {
    Request request;
    gfloat bands[NUM_EQ_BANDS];
} RequestEq;

typedef void (*RequestHandler)(Request *);

void jsonapi_read_request(Client *);
void jsonapi_set_request_handler(RequestType, RequestHandler);

void jsongen_send_packet(Client *, JsonPacket *);
void jsongen_broadcast_packet(Server *, JsonPacket *);

#endif /* jsonapi_h */
