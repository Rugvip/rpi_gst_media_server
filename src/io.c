
#include "io.h"
#include "jsonio.h"
#include "handler.h"

#include <gio/gunixinputstream.h>
#include <gio/gunixoutputstream.h>
#include <string.h>

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
        } else {
            g_warning("Unknown error reading input stream\n");
        }
    }
}

void io_init(Player *player)
{
    player->in = g_unix_input_stream_new(0, FALSE);
    player->out = g_unix_output_stream_new(1, FALSE);

    player->start_time = g_date_time_new_now_local();

    jsonio_set_input_handler(player, INPUT_INFO,   INPUT_HANDLER(handler_handle_info));
    jsonio_set_input_handler(player, INPUT_PLAY,   INPUT_HANDLER(handler_handle_play));
    jsonio_set_input_handler(player, INPUT_PAUSE,  INPUT_HANDLER(handler_handle_pause));
    jsonio_set_input_handler(player, INPUT_NEXT,   INPUT_HANDLER(handler_handle_next));
    jsonio_set_input_handler(player, INPUT_SEEK,   INPUT_HANDLER(handler_handle_seek));
    jsonio_set_input_handler(player, INPUT_VOLUME, INPUT_HANDLER(handler_handle_volume));
    jsonio_set_input_handler(player, INPUT_EQ,     INPUT_HANDLER(handler_handle_eq));
}

void io_start(Player *player)
{
    g_input_stream_read_async(player->in, player->buffer, INPUT_BUFFER_SIZE, G_PRIORITY_DEFAULT,
        NULL, (GAsyncReadyCallback) io_read_async, player);
}

void io_close(Player *player)
{
    GError *error = NULL;

    g_input_stream_close(player->in, NULL, &error);

    if (error) {
        g_warning("Error closing input stream: %s\n", error->message);
        g_clear_error(&error);
    }

    g_output_stream_close(player->out, NULL, &error);

    if (error) {
        g_warning("Error closing output stream: %s\n", error->message);
        g_clear_error(&error);
    }
}
