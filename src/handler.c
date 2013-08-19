
#include "handler.h"

#include "gst_utils.h"
#include "player.h"

void handler_handle_info(InputInfo *input)
{
    g_printerr("Got info input\n");

    Player *player;

    player = input->input.player;

    OutputPlaying output = {
        .output = {
            .player = player,
            .type = OUTPUT_PLAYING,
        },
        .song = player->source[0]->song,
        .duration = player_get_duration(player),
        .position = player_get_position(player),
    };

    jsonio_write(&output);
}

void handler_handle_play(InputPlay *input)
{
    g_printerr("Got play input %s/%s/%s %ld\n", input->song.artist
        ,input->song.album, input->song.name, input->time);

    if (!player_set_song(input->input.player, input->song)) {
        g_warning("Error handling input\n");
    }

    player_seek(input->input.player, input->time);
}

void handler_handle_pause(InputPause *input)
{
    g_printerr("Got pause input %ld\n", input->time);

    Player *player;

    player = input->input.player;
    player_pause(player);
    player_seek(player, input->time);
}

void handler_handle_next(InputNext *input)
{
    g_printerr("Got next input %s %s %s %ld\n", input->song.artist
        , input->song.album, input->song.name, input->time);
}

void handler_handle_seek(InputSeek *input)
{
    g_printerr("Got seek input %ld\n", input->time);
    player_seek(input->input.player, input->time);
}

void handler_handle_volume(InputVolume *input)
{
    g_printerr("Got volume input %f\n", input->volume);
}

void handler_handle_eq(InputEq *input)
{
    gint i;
    g_printerr("Got eq input\n");

    for (i = 0; i < NUM_EQ_BANDS; ++i) {
        g_printerr(" %3.2f", input->bands[i]);
    }

    g_printerr("\n");
}

static void duration_query_callback(Song song, gint64 duration, Player *player) {
    OutputPlaying output = {
        .output = {
            .type = OUTPUT_DURATION_RESULT,
            .player = player,
        },
        .song = song,
        .duration = duration,
    };

    jsonio_write(&output);

    g_free((gpointer) song.artist);
    g_free((gpointer) song.album);
    g_free((gpointer) song.name);
}

void handler_handle_duration_query(InputDurationQuery *input)
{
    g_printerr("Got duration query\n");

    Song song;
g_printerr("Handlin!");
    song.artist = g_strdup(input->song.artist);
    song.album = g_strdup(input->song.album);
    song.name = g_strdup(input->song.name);

    song_query_duration(song, (SongDurationQueryCallback) duration_query_callback, input->input.player);
}
