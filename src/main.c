
#include "common.h"
#include "server.h"
#include "player.h"

#include <json-glib/json-glib.h>

#define PORT 3264

void server_send_playback_status(Server *server)
{
    JsonPacket *packet;

    packet = jsongen_playing(server->player->mp3source->song,
        player_get_position(server->player), player_get_duration(server->player));
    jsonio_broadcast_packet(server, packet);
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

    g_print("Initializing server\n");
    server_init(server);
    g_print("Initializing player\n");
    player_init(server->player);

    g_timeout_add(60000, (GSourceFunc) server_print_connections, server);
    g_timeout_add(10000, (GSourceFunc) server_send_playback_status, server);

    g_print("Starting server\n");
    server_start(server, PORT);
    g_print("Starting player\n");
    player_start(server->player);
    return 0;
}
