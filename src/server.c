
#include "server.h"
#include "jsonio.h"
#include "handler.h"

#include <string.h>

#define IN_BUFFER_SIZE 1024

static void async_client_connection_read(GObject *obj, GAsyncResult *res, Client *client)
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

gboolean server_print_connections(Server *server)
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

static Client *client_new(Server *server, GSocketConnection *connection)
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

static gboolean on_client_connected(GSocketService    *service,
                                    GSocketConnection *connection,
                                    GObject           *source_object,
                                    Server            *server)
{
    Client *client;

    UNUSED(service);
    UNUSED(source_object);

    g_object_ref(G_OBJECT(connection));
    client = client_new(server, connection);
    g_ptr_array_add(server->clients, client);

    g_input_stream_read_async(client->in, client->buffer, IN_BUFFER_SIZE, G_PRIORITY_DEFAULT,
        NULL, (GAsyncReadyCallback) async_client_connection_read, client);
    return TRUE;
}

static void client_close_async(GSocketConnection *con, GAsyncResult *res, Client *client)
{
    GError *error;
    if (!g_io_stream_close_finish(G_IO_STREAM(con), res, &error)) {
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

static void client_free(Client *client)
{
    g_io_stream_close_async(G_IO_STREAM(client->connection), G_PRIORITY_DEFAULT,
        NULL, (GAsyncReadyCallback) client_close_async, client);
}

Server *server_new()
{
    Server *server;

    server = g_new0(Server, 1);

    server->clients = g_ptr_array_sized_new(10);
    g_ptr_array_set_free_func(server->clients, (GDestroyNotify) client_free);
    g_assert(server->clients);

    server->server_start_time = g_date_time_new_now_local();

    return server;
}

void server_init(Server *server)
{
    server->service = g_socket_service_new();
    g_assert(server->service);

    jsonio_set_request_handler(server, REQUEST_INFO,   REQUEST_HANDLER(handle_info_request));
    jsonio_set_request_handler(server, REQUEST_PLAY,   REQUEST_HANDLER(handle_play_request));
    jsonio_set_request_handler(server, REQUEST_PAUSE,  REQUEST_HANDLER(handle_pause_request));
    jsonio_set_request_handler(server, REQUEST_NEXT,   REQUEST_HANDLER(handle_next_request));
    jsonio_set_request_handler(server, REQUEST_SEEK,   REQUEST_HANDLER(handle_seek_request));
    jsonio_set_request_handler(server, REQUEST_VOLUME, REQUEST_HANDLER(handle_volume_request));
    jsonio_set_request_handler(server, REQUEST_EQ,     REQUEST_HANDLER(handle_eq_request));
}

void server_start(Server *server, gint port)
{
    GError *error = NULL;
    g_signal_connect(server->service, "incoming", G_CALLBACK(on_client_connected), server);
    g_socket_listener_add_inet_port(G_SOCKET_LISTENER(server->service), port, NULL, &error);
    g_socket_service_start(server->service);

    if (error) {
        g_error("Error setting up server: %s\n", error->message);
    }
}
