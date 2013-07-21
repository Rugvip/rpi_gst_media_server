#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>
#include <string.h>
#include <json-glib/json-glib.h>

#define PORT 3264

const gchar *MUSIC_DIR = "/home/pi/music";

#define UNUSED(obj) while(0){(void)obj;}

void handle_json_request(GSocketConnection *connection, JsonNode *root)
{
    GError *error = NULL;
    g_print("type: %s\n", json_node_type_name(root));

    JsonObject *rootObject = json_node_get_object(root);

    if (!rootObject) {
        g_warning("Warning: Json root node is not an object\n");
        return;
    }

    gchar *actionStr = json_object_get_string_member(rootObject, "action");

    if (!strncmp(actionStr, "start", 6)) {
        g_print("STARTING!\n");
    } else if (!strncmp(actionStr, "stop", 5)) {
        g_print("STOPPING!\n");
    }

    GOutputStream *ostream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    gchar *str = "{\"response\":\"ok\"}";
    gssize size = g_output_stream_write(ostream, str, strlen(str), NULL, &error);

    g_print("Sent %d bytes\n", size);
    if (error) {
        g_warning("Error writing to socket: %s\n", error->message);
        error = NULL;
    }
    g_socket_close(g_socket_connection_get_socket(connection), &error);
    if (error) {
        g_warning("Error closing socket: %s\n", error->message);
        error = NULL;
    }
}

gboolean on_client_connected(GSocketService    *server,
                             GSocketConnection *connection,
                             GObject           *source_object,
                             gpointer           user_data)
{
    GError *error = NULL;
    gboolean success;

    GSocketAddress *remote_socket_address = g_socket_connection_get_remote_address(connection, &error);
    if (error) {
        g_warning("Error getting remote address: %s\n", error->message);
        error = NULL;
    } else {
        GInetAddress *remote_inet_address;
        guint remote_port;
        gchar *remote_inet_address_str;
        
        g_object_get(G_OBJECT(remote_socket_address),
            "address", &remote_inet_address,
            "port", &remote_port,
            NULL);

        remote_inet_address_str = g_inet_address_to_string(remote_inet_address);
        g_print("New connection from %s:%d\n", remote_inet_address_str, remote_port);
        g_free(remote_inet_address_str);
    }

    GInputStream *istream = g_io_stream_get_input_stream(G_IO_STREAM(connection));

    JsonParser *parser = json_parser_new();
    success = json_parser_load_from_stream(parser, istream, NULL, &error);

    if (success) {
        handle_json_request(connection, json_parser_get_root(parser));
    } else {
        if (error) {
            g_warning("Json parser error: %s\n", error->message);
            error = NULL;
        } else {
            g_warning("Json parser failed without error\n");
        }
    }

    g_object_unref(G_OBJECT(parser));

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

    g_signal_connect(server, "incoming", G_CALLBACK(on_client_connected), NULL);

    g_socket_service_start(server);

    g_print("Entering main loop\n");

    main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(main_loop);
    return 0;
}
