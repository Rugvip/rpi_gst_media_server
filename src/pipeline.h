#ifndef pipeline_h
#define pipeline_h

#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>

#define UNUSED(obj) while(0){(void)obj;}

void seek(gint ms);

gint get_position();
void set_position(gint mstime);

void start();

typedef struct _UserData UserData;

typedef void (*Callback) (UserData*);

#define CALLBACK(func) ((Callback) func)

struct _UserData {
    GSocketConnection *connection;
    Callback callback;
    gint value;
    gpointer data;
};

gboolean play_file(const gchar *path, UserData *data);

#endif /* pipeline_h */
