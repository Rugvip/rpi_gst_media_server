#include "pipeline.h"

#include <string.h>
#include <json-glib/json-glib.h>

#define PORT 3264

const gchar *MUSIC_DIR = "/home/pi/music";

typedef struct {
    GSocketConnection *connection;
    GInputStream *in;
    GOutputStream *out;
    gchar *remote_address;
    guint remote_port;
} ClientData;

typedef struct {
    GPtrArray *clients;
} ConnectionData;

void write_json_response(GSocketConnection *connection, JsonNode *root)
{
    JsonGenerator *generator;
    GOutputStream *ostream;
    GError *error = NULL;

    generator = json_generator_new();
    json_generator_set_root(generator, root);

    ostream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    json_generator_to_stream(generator, ostream, NULL, &error);

    if (error) {
        g_warning("Error sending json stream: %s\n", error->message);
    }

    g_object_unref(G_OBJECT(generator));
}

void do_action_start_callback(UserData *data)
{
    JsonBuilder *builder;

    builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_add_string_value(
        json_builder_set_member_name(builder, "response"), "ok");
    json_builder_add_int_value(
        json_builder_set_member_name(builder, "duration"), data->value);
    json_builder_end_object(builder);

    write_json_response(data->connection, json_builder_get_root(builder));

    g_object_unref(G_OBJECT(builder));
    g_free(data);
}

void do_action_start(GSocketConnection *connection, JsonObject *root)
{
    g_object_ref(G_OBJECT(connection));
    JsonNode *fileNode = json_object_get_member(root, "file");

    UserData *data = g_new0(UserData, 1);
    data->connection = connection;
    data->callback =  do_action_start_callback;
    data->data = root;

    if (fileNode && json_node_get_value_type(fileNode) == G_TYPE_STRING) {
        play_file(json_node_get_string(fileNode), data);
    }
}

void do_action_stop(GSocketConnection *connection, JsonObject *root)
{
    UNUSED(root);
    JsonBuilder *builder;
    set_position(20000);

    builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_add_string_value(
        json_builder_set_member_name(builder, "response"), "ok");
    json_builder_end_object(builder);

    write_json_response(connection, json_builder_get_root(builder));
    g_object_unref(G_OBJECT(builder));
}

void handle_json_request(GSocketConnection *connection, JsonNode *root)
{
    g_print("type: %s\n", json_node_type_name(root));

    JsonObject *rootObject = json_node_get_object(root);

    if (!rootObject) {
        g_warning("Warning: Json root node is not an object\n");
        return;
    }

    JsonNode *actionNode = json_object_get_member(rootObject, "action");

    if (actionNode && json_node_get_value_type(actionNode) == G_TYPE_STRING) {
        const gchar *str = json_node_get_string(actionNode);
        g_print("Action: %s\n", str);
        if (!strncmp(str, "start", 6)) {
            do_action_start(connection, rootObject);
        } else if (!strncmp(str, "stop", 5)) {
            do_action_stop(connection, rootObject);
        }
    }
}

gpointer lebuf;

void readstuff(GObject *obj, GAsyncResult *res, gpointer user_data)
{
    GError *err = NULL;
    gssize len;
    GInputStream *istream = G_INPUT_STREAM(obj);

    len = g_input_stream_read_finish(istream, res, &err);

    g_print("Buf %u: %s\n", len, (gchar*) lebuf);

    g_input_stream_read_async(istream, lebuf, 1024, G_PRIORITY_DEFAULT,
        NULL, readstuff, user_data);
}

void client_data_print_interator(ClientData *client, ConnectionData *data)
{
    UNUSED(data);
    g_print("%s:%d\n", client->remote_address, client->remote_port);
}

gboolean connection_data_print_connections(ConnectionData *data)
{
    g_print("%d clients connected\n", data->clients->len);
    g_ptr_array_foreach(data->clients, (GFunc) client_data_print_interator, data);
    if (data->clients->len) {
        g_print("removing1 %d\n", data->clients->len);
        g_ptr_array_remove_index_fast(data->clients, 0);
        g_print("removing2 %d\n", data->clients->len);
    }
    return TRUE;
}

void read_client_connection_info(GSocketConnection *connection, ClientData *client)
{
    GError *error = NULL;
    GSocketAddress *remote_socket_address;
    remote_socket_address = g_socket_connection_get_remote_address(connection, &error);
    if (error) {
        g_warning("Error getting remote address: %s\n", error->message);
        error = NULL;
    } else {
        GInetAddress *remote_address;

        g_object_get(G_OBJECT(remote_socket_address),
            "address", &remote_address,
            "port", &client->remote_port,
            NULL);

        client->remote_address = g_inet_address_to_string(remote_address);
        g_print("Incoming connection %s:%d\n",
            client->remote_address, client->remote_port);
    }

    client->connection = connection;
    client->in = g_io_stream_get_input_stream(G_IO_STREAM(connection));
    client->out = g_io_stream_get_output_stream(G_IO_STREAM(connection));
}

gboolean on_client_connected(GSocketService    *server,
                             GSocketConnection *connection,
                             GObject           *source_object,
                             ConnectionData    *data)
{
    GError *error = NULL;
    gboolean success;
    ClientData *client;

    UNUSED(server);
    UNUSED(source_object);

    g_object_ref(G_OBJECT(connection));
    client = g_new0(ClientData, 1);
    read_client_connection_info(connection, client);
    g_print("adding1 %d\n", data->clients->len);
    g_ptr_array_add(data->clients, client);
    g_print("adding2 %d\n", data->clients->len);


    JsonParser *parser = json_parser_new();
    error = NULL;
    success = FALSE;
    while (0) {
        success = json_parser_load_from_stream(parser, client->in, NULL, &error);
    }

    g_input_stream_read_async(client->in, lebuf, 1024, G_PRIORITY_DEFAULT,
        NULL, readstuff, connection);

    if (success) {
        handle_json_request(connection, json_parser_get_root(parser));
    } else {
        if (error) {
            g_warning("Json parser error: %s\n", error->message);
        } else {
            g_warning("Json parser failed without error\n");
        }
    }

    g_object_unref(G_OBJECT(parser));
    return TRUE;
}

void on_client_data_connection_closed(GSocketConnection *connection,
    GAsyncResult *result, ClientData *client)
{
    GError *error;
    if (!g_io_stream_close_finish(G_IO_STREAM(connection), result, &error)) {
        if (error) {
            g_warning("Error closing connection: %s\n", error->message);
        } else {
            g_warning("Json parser failed without error\n");
        }
    }
    g_print("Closing connection to %s:%d", client->remote_address, client->remote_port);

    g_free(client->remote_address);
    client->remote_address = NULL;
    client->remote_port = 0;
}

void client_data_free(ClientData *client)
{
    g_print("freen\n");
    g_io_stream_close_async(G_IO_STREAM(client->connection), G_PRIORITY_DEFAULT,
        NULL, (GAsyncReadyCallback) on_client_data_connection_closed, client);
}

void connection_data_init(ConnectionData *data)
{
    g_print("init\n");
    data->clients = g_ptr_array_new_with_free_func((GDestroyNotify) client_data_free);
}

void setup_server(ConnectionData *data)
{
    GError *error = NULL;
    GSocketService *server;

    server = g_socket_service_new();

    g_socket_listener_add_inet_port(G_SOCKET_LISTENER(server), PORT, NULL, &error);
    g_signal_connect(server, "incoming", G_CALLBACK(on_client_connected), &data);
    g_socket_service_start(server);

    if (error) {
        g_error(error->message);
    }
}

int main(int argc, const char *argv[])
{
    g_print("Initializing in %s\n", argv[0]);
    if (argc > 1) {
        g_print("Wai u passing arguments?");
    }

    ConnectionData *data = g_new0(ConnectionData, 1);
    lebuf = g_malloc0(1024);

    connection_data_init(data);
    setup_server(data);

    g_timeout_add(20000, (GSourceFunc) connection_data_print_connections, data);

    start();
    return 0;
}
