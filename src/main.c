#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>

#define PORT 3264

gboolean client_connected(GSocketService    *server,
                          GSocketConnection *connection,
                          GObject           *source_object,
                          gpointer           user_data)
{
    g_print("Connection!\n");

    GInputStream *istream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
    gchar message[1024] = {0};
    g_input_stream_read(istream, message, 1024, NULL, NULL);
    g_print("Message was: \"%s\"\n", message);

    g_object_set(connection, "graceful-disconnect", TRUE, NULL);
    return TRUE;
}

int main(int argc, char const *argv[])
{
    g_print("Hello wat!\n");
    GMainLoop* main_loop;
    GSocketService *server;
    GError *error = NULL;

    server = g_socket_service_new();
    g_socket_listener_add_inet_port(G_SOCKET_LISTENER(server), PORT, NULL, &error);

    if (error) {
        g_error(error->message);
    }

    g_signal_connect(server, "incoming", G_CALLBACK(client_connected), NULL);

    g_socket_service_start(server);

    g_print("Entering main loop\n");

    main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(main_loop);
    return 0;
}
