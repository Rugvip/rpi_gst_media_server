#ifndef pipeline_h
#define pipeline_h

#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>

#define UNUSED(obj) while(0){(void)obj;}

#define CLIENT(obj) ((Client *) (obj))
#define SERVER(obj) ((Server *) (obj))

void seek(gint ms);

gint get_duration();
gint get_position();
void set_position(gint mstime);

void start();

typedef struct _UserData UserData;

typedef void (*Callback) (UserData*);

#define CALLBACK(func) ((Callback) func)

#define NUM_EQ_BANDS 10
#define CLIENT_BUFFER_SIZE 1024

typedef struct {
    gchar *artist;
    gchar *album;
    gchar *song;
} Song;

typedef struct {
    GPtrArray *clients;
    GDateTime *server_start_time;
} Server;

typedef struct {
    GSocketConnection *connection;
    GInputStream *in;
    GOutputStream *out;
    gchar *remote_address;
    guint remote_port;
    GDateTime *connection_time;
    Server *server;
    gchar buffer[CLIENT_BUFFER_SIZE];
    gssize buffer_len;
} Client;

struct _UserData {
    Client *client;
    Callback callback;
    gint value;
    gpointer data;
};

gboolean play_file(const gchar *path, UserData *data);

#endif /* pipeline_h */
