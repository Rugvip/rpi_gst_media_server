#ifndef common_h
#define common_h

#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <gst/gst.h>

#define UNUSED(obj) while(0){(void)obj;}

#define PLAYER(obj) ((Player *) (obj))
#define CALLBACK(func) ((Callback) func)

#define NUM_EQ_BANDS 10

#define MUSIC_DIR "/home/rugvip/Music"

typedef struct _MP3Source MP3Source;
typedef struct _Player Player;
typedef struct _Song Song;

struct _Song {
    gchar *artist;
    gchar *album;
    gchar *name;
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

    GDateTime *start_time;

    gint source_id;
};

/*
// Input to send information
typedef struct {
    Input input;
} InputInfo;

// Song to play starting at time
typedef struct {
    Input input;
    Song song;
    gint64 time;
} InputPlay;

// Pause the playing song and seek to time
typedef struct {
    Input input;
    gint64 time;
} InputPause;

// The next song to be played
typedef struct {
    Input input;
    Song song;
    gint64 time;
} InputNext;

// Seek to a specific time
typedef struct {
    Input input;
    gint64 time;
} InputSeek;

// Set volume
typedef struct {
    Input input;
    gdouble volume;
} InputVolume;

// Set equalizer settings
typedef struct {
    Input input;
    gdouble bands[NUM_EQ_BANDS];
} InputEq;

// Query the duration of a song
typedef struct {
    Input input;
    Song song;
} InputDurationQuery;


typedef struct {
    OutputType type;
    Player *player;
} Output;

typedef struct {
    Output output;
    Song song;
    gint64 duration;
    gint64 position;
} OutputPlaying;

typedef struct {
    Output output;
    gint64 time;
} OutputPaused;

typedef struct {
    Output output;
    gdouble bands[NUM_EQ_BANDS];
} OutputEq;

typedef struct {
    Output output;
    gdouble volume;
} OutputVolume;

typedef struct {
    Output output;
    Song song;
    gint64 duration;
    gint64 position;
} OutputInfo;

typedef struct {
    Output output;
    Song song;
    gint64 duration;
} OutputDurationResult;

*/

#endif /* common_h */
