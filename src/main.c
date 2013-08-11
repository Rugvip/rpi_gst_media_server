
#include "common.h"
#include "server.h"
#include "player.h"

#include <json-glib/json-glib.h>

#define PORT 3264

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

    server_start(server, PORT);
    player_start(server->player);
    return 0;
}
