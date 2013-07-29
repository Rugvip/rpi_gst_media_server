#include "pipeline.h"

#include "jsonio.h"

#include <string.h>
#include <json-glib/json-glib.h>

#define PORT 3264
#define IN_BUFFER_SIZE 1024

const gchar *MUSIC_DIR = "/home/rugvip/music";

void handle_info_request(RequestInfo *request)
{
    UNUSED(request);
    g_print("Got info request\n");
}

void handle_play_request(RequestPlay *request)
{
    g_print("Got play request %s/%s/%s %ld\n", request->song.artist
        , request->song.album, request->song.song, request->time);
}

void handle_pause_request(RequestPause *request)
{
    g_print("Got pause request %ld\n", request->time);
}

void handle_next_request(RequestNext *request)
{
    g_print("Got next request %s %s %s %ld\n", request->song.artist
        , request->song.album, request->song.song, request->time);
}

void handle_seek_request(RequestSeek *request)
{
    g_print("Got seek request %ld\n", request->time);
}

void handle_volume_request(RequestVolume *request)
{
    g_print("Got volume request %f\n", request->volume);
}

void handle_eq_request(RequestEq *request)
{
    gint i;
    g_print("Got eq request");

    for (i = 0; i < NUM_EQ_BANDS; ++i) {
        g_print(" %3.2f", request->bands[i]);
    }

    g_print("\n");
}


void do_action_start_callback(UserData *data)
{
    jsonio_send_packet(data->client, jsongen_playing((Song){NULL, NULL, NULL}, data->value, 0));
    g_free(data);
}

void do_action_start(Client *client, JsonObject *root)
{
    JsonNode *fileNode = json_object_get_member(root, "file");

    UserData *data = g_new0(UserData, 1);
    data->client = client;
    data->callback =  do_action_start_callback;
    data->data = root;

    if (fileNode && json_node_get_value_type(fileNode) == G_TYPE_STRING) {
        play_file(json_node_get_string(fileNode), data);
    }
}

void async_client_connection_read(GObject *obj, GAsyncResult *res, Client *client)
{
    GError *err = NULL;
    gssize len;
    GInputStream *istream = G_INPUT_STREAM(obj);

    len = g_input_stream_read_finish(istream, res, &err);

    if (len > 0) {
        client->buffer_len = len;

        if (len > CLIENT_BUFFER_SIZE / 2) {
            g_print("Received buffer with size %ld, DDOS!?\n", len);
        }

        jsonio_read_request(client);

        memset(client->buffer, 0, len);
        g_input_stream_read_async(istream, client->buffer, CLIENT_BUFFER_SIZE, G_PRIORITY_DEFAULT,
            NULL, (GAsyncReadyCallback) async_client_connection_read, client);
    } else {
        if (err) {
            g_warning("Error reading input stream, %s\n", err->message);
        }
        g_ptr_array_remove(client->server->clients, client);
    }
}

void server_send_playback_status(Server *server)
{
    jsonio_broadcast_packet(server, jsongen_playing((Song){NULL, NULL, NULL}, get_position(), get_duration()));
}

gboolean server_data_print_connections(Server *server)
{
    gint i;
    GPtrArray *clients = server->clients;
    gint len = server->clients->len;

    g_print("%d clients connected\n", len);
    for (i = 0; i < len; ++i) {
        Client *client;
        client = g_ptr_array_index(clients, i);
        g_print("%s:%d\n", client->remote_address, client->remote_port);
    }
    return TRUE;
}

Client *client_data_new(Server *server, GSocketConnection *connection)
{
    GError *error = NULL;
    Client *client;
    GSocketAddress *remote_socket_address;

    client = g_new0(Client, 1);

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

    return client;
}

gboolean on_client_connected(GSocketService    *service,
                             GSocketConnection *connection,
                             GObject           *source_object,
                             Server        *server)
{
    Client *client;

    UNUSED(service);
    UNUSED(source_object);

    g_object_ref(G_OBJECT(connection));
    client = client_data_new(server, connection);
    g_ptr_array_add(server->clients, client);

    g_input_stream_read_async(client->in, client->buffer, IN_BUFFER_SIZE, G_PRIORITY_DEFAULT,
        NULL, (GAsyncReadyCallback) async_client_connection_read, client);
    return TRUE;
}

void on_client_data_connection_closed(GSocketConnection *connection,
    GAsyncResult *result, Client *client)
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
    g_print("Connected for %lud %luh %lum %lus\n",
        diff / G_TIME_SPAN_DAY,
        (diff / G_TIME_SPAN_HOUR) % 24,
        (diff / G_TIME_SPAN_MINUTE) % 60,
        (diff / G_TIME_SPAN_SECOND) % 60);

    g_free(client->remote_address);
    client->remote_address = NULL;
    client->remote_port = 0;
    g_date_time_unref(client->connection_time);
}

void client_data_free(Client *client)
{
    g_io_stream_close_async(G_IO_STREAM(client->connection), G_PRIORITY_DEFAULT,
        NULL, (GAsyncReadyCallback) on_client_data_connection_closed, client);
}

Server *server_data_new()
{
    Server *server;

    server = g_new0(Server, 1);

    server->clients = g_ptr_array_sized_new(10);
    g_ptr_array_set_free_func(server->clients, (GDestroyNotify) client_data_free);
    g_assert(server->clients);

    server->server_start_time = g_date_time_new_now_local();

    return server;
}

void setup_server(Server *server)
{
    GError *error = NULL;
    GSocketService *service;

    service = g_socket_service_new();
    g_assert(service);

    jsonio_set_request_handler(REQUEST_INFO,   REQUEST_HANDLER(handle_info_request));
    jsonio_set_request_handler(REQUEST_PLAY,   REQUEST_HANDLER(handle_play_request));
    jsonio_set_request_handler(REQUEST_PAUSE,  REQUEST_HANDLER(handle_pause_request));
    jsonio_set_request_handler(REQUEST_NEXT,   REQUEST_HANDLER(handle_next_request));
    jsonio_set_request_handler(REQUEST_SEEK,   REQUEST_HANDLER(handle_seek_request));
    jsonio_set_request_handler(REQUEST_VOLUME, REQUEST_HANDLER(handle_volume_request));
    jsonio_set_request_handler(REQUEST_EQ,     REQUEST_HANDLER(handle_eq_request));

    g_socket_listener_add_inet_port(G_SOCKET_LISTENER(service), PORT, NULL, &error);
    g_signal_connect(service, "incoming", G_CALLBACK(on_client_connected), server);
    g_socket_service_start(service);

    if (error) {
        g_error("Error setting up server: %s\n", error->message);
    }
}

int main(int argc, const char *argv[])
{
    g_print("Initializing %s\n", argv[0]);
    if (argc > 1) {
        g_print("Wai u passing arguments?");
    }

    Server *server;

    server = server_data_new();

    setup_server(server);

    g_timeout_add(60000, (GSourceFunc) server_data_print_connections, server);

    g_timeout_add(10000, (GSourceFunc) server_send_playback_status, server);

    start();
    return 0;
}
