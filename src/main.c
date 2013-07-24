#include "pipeline.h"

#include <string.h>
#include <json-glib/json-glib.h>

#define PORT 3264
#define IN_BUFFER_SIZE 1024

#define warn_if_error(str, error) if (error) { g_warning(str, error->message); }

const gchar *MUSIC_DIR = "/home/pi/music";

typedef struct {
    GPtrArray *clients;
    GDateTime *server_start_time;
} ServerData;

typedef struct {
    GSocketConnection *connection;
    GInputStream *in;
    GOutputStream *out;
    gchar *remote_address;
    guint remote_port;
    GDateTime *connection_time;
    ServerData *server;
    guchar *buffer;
} ClientData;

void write_json_response(GSocketConnection *connection, JsonNode *root)
{
    JsonGenerator *generator;
    GOutputStream *ostream;
    GError *error = NULL;

    generator = json_generator_new();
    json_generator_set_root(generator, root);

    ostream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    json_generator_to_stream(generator, ostream, NULL, &error);

    warn_if_error("Error sending json stream: %s\n", error);

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

void on_client_connection_read(GObject *obj, GAsyncResult *res, ClientData *client)
{
    GError *err = NULL;
    gssize len;
    GInputStream *istream = G_INPUT_STREAM(obj);

    len = g_input_stream_read_finish(istream, res, &err);

    if (len > 0) {
        g_print("Buf %u: %s\n", len, client->buffer);

        memset(client->buffer, 0, len);
        g_input_stream_read_async(istream, client->buffer, 1024, G_PRIORITY_DEFAULT,
            NULL, (GAsyncReadyCallback) on_client_connection_read, client);
    } else {
        if (err) {
            g_warning("Error reading input stream, %s\n", err->message);
        }
        g_ptr_array_remove(client->server->clients, client);
    }
}

void client_data_print_interator(ClientData *client, ServerData *server)
{
    UNUSED(server);
    g_print("%s:%d\n", client->remote_address, client->remote_port);
}

gboolean server_data_print_connections(ServerData *server)
{
    g_print("%d clients connected\n", server->clients->len);
    g_ptr_array_foreach(server->clients, (GFunc) client_data_print_interator, server);
    return TRUE;
}

ClientData *client_data_new(ServerData *server, GSocketConnection *connection)
{
    GError *error = NULL;
    ClientData *client;
    GSocketAddress *remote_socket_address;

    client = g_new0(ClientData, 1);

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
    client->connection_time = g_date_time_new_now_local();
    client->server = server;
    client->buffer = g_malloc0(IN_BUFFER_SIZE);

    return client;
}

gboolean on_client_connected(GSocketService    *service,
                             GSocketConnection *connection,
                             GObject           *source_object,
                             ServerData        *server)
{
    GError *error = NULL;
    gboolean success;
    ClientData *client;

    UNUSED(service);
    UNUSED(source_object);

    g_object_ref(G_OBJECT(connection));
    client = client_data_new(server, connection);
    g_ptr_array_add(server->clients, client);

    g_input_stream_read_async(client->in, client->buffer, IN_BUFFER_SIZE, G_PRIORITY_DEFAULT,
        NULL, (GAsyncReadyCallback) on_client_connection_read, client);


    while (0) {
        JsonParser *parser = json_parser_new();
        error = NULL;
        success = json_parser_load_from_stream(parser, client->in, NULL, &error);

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
    }
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

    GDateTime *now = g_date_time_new_now_local();
    GTimeSpan diff = g_date_time_difference(now, client->connection_time);
    g_print("Closing connection to %s:%d\n", client->remote_address, client->remote_port);
    g_print("Connected for %llud %lluh %llum %llus\n",
        diff / G_TIME_SPAN_DAY,
        (diff / G_TIME_SPAN_HOUR) % 24,
        (diff / G_TIME_SPAN_MINUTE) % 60,
        (diff / G_TIME_SPAN_SECOND) % 60);

    g_free(client->remote_address);
    client->remote_address = NULL;
    client->remote_port = 0;
    g_date_time_unref(client->connection_time);
}

void client_data_free(ClientData *client)
{
    g_io_stream_close_async(G_IO_STREAM(client->connection), G_PRIORITY_DEFAULT,
        NULL, (GAsyncReadyCallback) on_client_data_connection_closed, client);
}

ServerData *server_data_new()
{
    ServerData *server;

    server = g_new0(ServerData, 1);

    server->clients = g_ptr_array_sized_new(10);
    g_ptr_array_set_free_func(server->clients, (GDestroyNotify) client_data_free);
    g_assert(server->clients);

    server->server_start_time = g_date_time_new_now_local();

    return server;
}

void setup_server(ServerData *server)
{
    GError *error = NULL;
    GSocketService *service;

    service = g_socket_service_new();
    g_assert(service);

    g_socket_listener_add_inet_port(G_SOCKET_LISTENER(service), PORT, NULL, &error);
    g_signal_connect(service, "incoming", G_CALLBACK(on_client_connected), server);
    g_socket_service_start(service);

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

    ServerData *server;

    server = server_data_new();

    setup_server(server);

    g_timeout_add(60000, (GSourceFunc) server_data_print_connections, server);

    start();
    return 0;
}
