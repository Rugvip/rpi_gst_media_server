#ifndef pipeline_h
#define pipeline_h

#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>

#define UNUSED(obj) while(0){(void)obj;}

void seek(gint ms);

gint get_duration();
gint get_position();
void set_position(gint mstime);

void start();

typedef struct _UserData UserData;

typedef void (*Callback) (UserData*);

#define CALLBACK(func) ((Callback) func)

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
    gchar *buffer;
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
