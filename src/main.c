
#include "common.h"
#include "server.h"
#include "player.h"

#include <json-glib/json-glib.h>

#define PORT 3264

void server_send_playback_status(Server *server)
{
    ResponsePlaying response = {
        .song = server->player->source[0]->song,
        .duration = player_get_duration(server->player),
        .position = player_get_position(server->player),
    };

    jsonio_broadcast_packet(server, jsonio_response_playing_packet(&response));
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

int main(int argc, const char *argv[])
{
    g_print("Starting %s\n", argv[0]);
    if (argc > 1) {
        g_print("Wai u passing arguments?");
    }

    Server *server;

    server = server_new();
    server->player = player_new(server);

    server_init(server);
    player_init(server->player);

    g_timeout_add(60000, (GSourceFunc) server_print_connections, server);
    g_timeout_add(10000, (GSourceFunc) server_send_playback_status, server);

    server_start(server, PORT);
    player_start(server->player);
    return 0;
}
