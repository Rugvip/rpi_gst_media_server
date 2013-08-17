
#include "server.h"
#include "jsonio.h"
#include "handler.h"

#include <string.h>

#define IN_BUFFER_SIZE 1024

static void io_read_async(GObject *obj, GAsyncResult *res, Player *player)
{
    GError *err = NULL;
    gssize len;
    GInputStream *istream = G_INPUT_STREAM(obj);

    len = g_input_stream_read_finish(istream, res, &err);

    if (len > 0) {
        player->buffer_len = len;

        if (len > INPUT_BUFFER_SIZE / 2) {
            g_printerr("Received buffer with size %ld, DDOS!?\n", len);
        }

        jsonio_read(player);

        memset(player->buffer, 0, len);
        g_input_stream_read_async(istream, player->buffer, INPUT_BUFFER_SIZE, G_PRIORITY_DEFAULT,
            NULL, (GAsyncReadyCallback) io_read_async, player);
    } else {
        if (err) {
            g_warning("Error reading input stream, %s\n", err->message);
        }
        g_ptr_array_remove(player->server->players, player);
    }
}

void io_init(Player *player)
{
    player->in = g_io_stream_get_input_stream(G_IO_STREAM(connection));
    player->out = g_io_stream_get_output_stream(G_IO_STREAM(connection));

    g_input_stream_read_async(player->in, player->buffer, IN_BUFFER_SIZE, G_PRIORITY_DEFAULT,
        NULL, (GAsyncReadyCallback) io_read_async, player);

    server->server_start_time = g_date_time_new_now_local();

    jsonio_set_input_handler(server, INPUT_INFO,   INPUT_HANDLER(handler_handle_info));
    jsonio_set_input_handler(server, INPUT_PLAY,   INPUT_HANDLER(handler_handle_play));
    jsonio_set_input_handler(server, INPUT_PAUSE,  INPUT_HANDLER(handler_handle_pause));
    jsonio_set_input_handler(server, INPUT_NEXT,   INPUT_HANDLER(handler_handle_next));
    jsonio_set_input_handler(server, INPUT_SEEK,   INPUT_HANDLER(handler_handle_seek));
    jsonio_set_input_handler(server, INPUT_VOLUME, INPUT_HANDLER(handler_handle_volume));
    jsonio_set_input_handler(server, INPUT_EQ,     INPUT_HANDLER(handler_handle_eq));

}

static void io_close_async(GSocketConnection *con, GAsyncResult *res, Player *player)
{
    GError *error = NULL;
    if (!g_io_stream_close_finish(G_IO_STREAM(con), res, &error)) {
        if (error) {
            g_warning("Error closing connection: %s\n", error->message);
        } else {
            g_warning("Json parser failed without error\n");
        }
    }


void io_close(Player *player)
{
    g_io_stream_close_async(G_IO_STREAM(player->connection), G_PRIORITY_DEFAULT,
        NULL, (GAsyncReadyCallback) io_close_async, player);
}
