
#include "common.h"
#include "io.h"
#include "player.h"

#include <json-glib/json-glib.h>

int main(int argc, const char *argv[])
{
    Player *player;

    g_printerr("Starting %s\n", argv[0]);
    if (argc > 1) {
        g_printerr("Wai u passing arguments?");
    }

    player = player_init(player_alloc());

    io_init(player);

    io_start(player);
    player_start(player);
    return 0;
}
