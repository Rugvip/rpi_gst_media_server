
#include "common.h"
#include "io.h"
#include "player.h"

#include <json-glib/json-glib.h>

#define PORT 3264

int main(int argc, const char *argv[])
{
    g_printerr("Starting %s\n", argv[0]);
    if (argc > 1) {
        g_printerr("Wai u passing arguments?");
    }

    Player *player;

    player = player_new();

    io_init(player);
    player_init(player);

    player_start(player);
    io_start(player);
    return 0;
}
