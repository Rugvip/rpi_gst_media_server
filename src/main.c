
#include "common.h"
#include "player.h"

#include <json-glib/json-glib.h>

int main(int argc, const char *argv[])
{
    Player *player;

    g_printerr("Starting %s\n", argv[0]);
    if (argc > 1) {
        g_printerr("Wai u passing arguments?");
    }

    char *args[] = {
        "mp3net",
        "--gst-debug-no-color",
        "--gst-debug-level=2"
    };

    player = player_init(player_alloc(),
        sizeof(args)/sizeof(char *), (char **) args);

    player_start(player);
    return 0;
}
