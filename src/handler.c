
#include "handler.h"
#include "player.h"

void handle_info_request(RequestInfo *request)
{
    UNUSED(request);
    g_print("Got info request\n");
}

void handle_play_request(RequestPlay *request)
{
    g_print("Got play request %s/%s/%s %ld\n", request->song.artist
        ,request->song.album, request->song.name, request->time);

    if (!player_set_song(request->request.client->server->player, request->song)) {
        g_warning("Error handling request\n");
    }

    player_set_position(request->request.client->server->player, request->time);
    g_print("Pos: %ld\n", player_get_position(request->request.client->server->player));
    g_print("Dur: %ld\n", player_get_duration(request->request.client->server->player));
}

void handle_pause_request(RequestPause *request)
{
    g_print("Got pause request %ld\n", request->time);
}

void handle_next_request(RequestNext *request)
{
    g_print("Got next request %s %s %s %ld\n", request->song.artist
        , request->song.album, request->song.name, request->time);
}

void handle_seek_request(RequestSeek *request)
{
    g_print("Got seek request %ld\n", request->time);
    player_set_position(request->request.client->server->player, request->time);
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
