#ifndef common_h
#define common_h

#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <gst/gst.h>

#define UNUSED(obj) while(0){(void)obj;}

#define PLAYER(obj) ((Player *) (obj))
#define CALLBACK(func) ((Callback) func)
#define REQUEST_HANDLER(obj) ((RequestHandler) (obj))

#define NUM_EQ_BANDS 10
#define CLIENT_BUFFER_SIZE 1024

const gchar *const MUSIC_DIR;

typedef enum {
    REQUEST_INFO,
    REQUEST_PLAY,
    REQUEST_PAUSE,
    REQUEST_NEXT,
    REQUEST_SEEK,
    REQUEST_VOLUME,
    REQUEST_EQ,
    NUM_REQUEST_TYPES,
} RequestType;

typedef struct _MP3Source MP3Source;
typedef struct _Player Player;
typedef struct _Song Song;
typedef struct _JsonPacket JsonPacket;
typedef struct _Request Request;

typedef void (*RequestHandler)(Request *);

struct _JsonPacket {
    void *priv;
};

struct _Song {
    const gchar *artist;
    const gchar *album;
    const gchar *name;
};

struct _MP3Source {
    GstElement *filesrc, *parser, *decoder;
    GstElement *bin;
    GstPad *pad;
    Song song;
    GstPad *adderPad;
};

struct _Player {
    GstElement *pipeline;
    MP3Source *source[2];
    GstElement *adder, *volume, *equalizer, *sink;

    GMainLoop *main_loop;
    guint bus_watch_id;

    GInputStream *in;
    GOutputStream *out;
    gchar buffer[CLIENT_BUFFER_SIZE];
    gssize buffer_len;

    GDateTime *server_start_time;
    RequestHandler handlers[NUM_REQUEST_TYPES];

    gint source_id;
};

struct _Request {
    RequestType type;
    Player *player;
};

/* Request to send information */
typedef struct {
    Request request;
} RequestInfo;

/* Song to play starting at time */
typedef struct {
    Request request;
    Song song;
    gint64 time;
} RequestPlay;

/* Pause the playing song and seek to time */
typedef struct {
    Request request;
    gint64 time;
} RequestPause;

/* The next song to be played */
typedef struct {
    Request request;
    Song song;
    gint64 time;
} RequestNext;

/* Seek to a specific time */
typedef struct {
    Request request;
    gint64 time;
} RequestSeek;

/* Set volume */
typedef struct {
    Request request;
    gdouble volume;
} RequestVolume;

/* Set equalizer settings */
typedef struct {
    Request request;
    gdouble bands[NUM_EQ_BANDS];
} RequestEq;



typedef struct {
    Song song;
    gint64 duration;
    gint64 position;
} ResponsePlaying;

typedef struct {
    gint64 time;
} ResponsePaused;

typedef struct {
    gdouble bands[NUM_EQ_BANDS];
} ResponseEq;

typedef struct {
    gdouble volume;
} ResponseVolume;

typedef struct {
    Song song;
    gint64 duration;
    gint64 position;
} ResponseInfo;

#endif /* common_h */
