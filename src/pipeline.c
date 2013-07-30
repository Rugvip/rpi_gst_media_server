#include "pipeline.h"

const gchar *const MUSIC_DIR = "/home/rugvip/music";

gint64 player_get_duration(Player *player)
{
    static GstQuery *query = NULL;

    if (!query) {
        query = gst_query_new_duration(GST_FORMAT_TIME);
    }

    if (gst_element_query(player->mp3source->parser, query)) {
        gint64 duration;
        gst_query_parse_duration(query, NULL, &duration);
        return GST_TIME_AS_MSECONDS(duration);
    } else {
        return -1;
    }
}

gint64 player_get_position(Player *player)
{
    static GstQuery *query = NULL;

    if (!query) {
        query = gst_query_new_position(GST_FORMAT_TIME);
    }

    if (gst_element_query(player->mp3source->parser, query)) {
        gint64 position;
        gst_query_parse_position(query, NULL, &position);
        return GST_TIME_AS_MSECONDS(position);
    } else {
        return -1;
    }
}

void player_set_position(Player *player, gint ms)
{
    gboolean ret;

    ret = gst_element_seek_simple(player->pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, ms * GST_MSECOND);
    if(ret) {
        g_print("seek done.");
    } else {
        g_warning("seek failed");
    }
}

static gboolean timeout_duration_query(Server *server)
{
    static int i = 0;
    gint64 duration = player_get_duration(server->player);
    g_print("testtimeout %d\n", ++i);
    if (duration < 0) {
        return TRUE;
    }

    JsonPacket *packet;
    gint64 position;

    position = player_get_position(server->player);

    position = position < 0 ? 0 : position;

    packet = jsongen_playing(server->player->mp3source->song, duration, position);
    jsonio_broadcast_packet(server, packet);

    return FALSE;
}

gboolean player_set_song(Player *player, Song song)
{
    GstStateChangeReturn ret;
    gst_element_set_state(player->pipeline, GST_STATE_NULL); // FIXME: Needed?

    gchar *path;

    path = g_build_filename(MUSIC_DIR, song.artist, song.album, song.name, NULL);

    g_object_set(G_OBJECT(player->mp3source->filesrc), "location", path, NULL);
    ret = gst_element_set_state(player->pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_warning("State change failed");
        g_free(path);
        return FALSE;
    }
    g_timeout_add(10, (GSourceFunc)timeout_duration_query, player->server);
    g_print("Now playing: %s\n", path);

    g_free(path);

    return TRUE;
}

void handle_info_request(RequestInfo *request)
{
    UNUSED(request);
    g_print("Got info request\n");
}

void handle_play_request(RequestPlay *request)
{
    g_print("Got play request %s/%s/%s %ld\n", request->song.artist
        ,request->song.album, request->song.name, request->time);

    if (!player_set_song(request->request.client->server->player , request->song)) {
        g_warning("Error handling request\n");
    }
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

static gboolean bus_call(GstBus *bus, GstMessage *msg, Player *player)
{
    UNUSED(bus);

    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
        g_print("End of stream\n");
        gst_element_set_state(player->pipeline, GST_STATE_PAUSED);
        break;

    case GST_MESSAGE_ERROR: {
            gchar *debug;
            GError *error;

            gst_message_parse_error(msg, &error, &debug);
            g_free(debug);

            g_printerr("Error: %s\n", error->message);
            g_error_free(error);
            break;
        }
    default:
        break;
    }

    return TRUE;
}

static MP3Source *mp3_source_new()
{
    MP3Source *source;

    source = g_new0(MP3Source, 1);

    return source;
}

Player *player_new(Server *server)
{
    Player *player;

    player = g_new0(Player, 1);

    player->mp3source = mp3_source_new();
    player->server = server;

    return player;
}

static void mp3_source_init(MP3Source *src)
{
    src->filesrc = gst_element_factory_make("filesrc"        , "file-src");
    g_assert(src->filesrc);
    src->parser  = gst_element_factory_make("mpegaudioparse" , "mpeg-audio-parser");
    g_assert(src->parser);
    src->decoder = gst_element_factory_make("mpg123audiodec" , "mpeg-audio-decoder");
    g_assert(src->decoder);
    src->bin = gst_bin_new("mp3-src-bin");
    g_assert(src->bin);

    gst_bin_add_many(GST_BIN(src->bin), src->filesrc, src->parser, src->decoder, NULL);
    gst_element_link_many(src->filesrc, src->parser, src->decoder, NULL);

    GstPad *pad = gst_element_get_static_pad(src->decoder, "src");
    gst_element_add_pad(src->bin, gst_ghost_pad_new("src", pad));
    gst_object_unref(GST_OBJECT (pad));
}

void player_init(Player *player)
{
    GstBus *bus;

    char *args[] = {
        "mp3net",
        "--gst-debug-no-color",
        "--gst-debug-level=3"
    };
    char **argv = args;
    int argc = sizeof(args)/sizeof(gchar *);

    gst_init(&argc, &argv);

    player->main_loop = g_main_loop_new(NULL, FALSE);

    player->pipeline  = gst_pipeline_new("mediaplayer");
    g_assert(player->pipeline);
    player->volume    = gst_element_factory_make("volume" , "volume");
    g_assert(player->volume);
    player->equalizer = gst_element_factory_make("equalizer-10bands" , "equalizer");
    g_assert(player->equalizer);
    player->sink      = gst_element_factory_make("alsasink" , "alsa-sink");
    g_assert(player->sink);

    mp3_source_init(player->mp3source);

    gst_bin_add_many(GST_BIN(player->pipeline),
        player->mp3source->bin,
        player->volume,
        player->equalizer,
        player->sink,
        NULL);

    gst_element_link_many(
        player->mp3source->bin,
        player->volume,
        player->equalizer,
        player->sink,
        NULL);

    bus = gst_pipeline_get_bus(GST_PIPELINE(player->pipeline));
    player->bus_watch_id = gst_bus_add_watch(bus, (GstBusFunc) bus_call, player);
    gst_object_unref(bus);

    g_object_set(G_OBJECT(player->volume), "volume", 2.0, NULL);
    g_object_set(G_OBJECT(player->equalizer), "band0", 4.0, NULL);
    g_object_set(G_OBJECT(player->equalizer), "band1", 3.0, NULL);
    g_object_set(G_OBJECT(player->equalizer), "band2", 2.0, NULL);
    g_object_set(G_OBJECT(player->equalizer), "band3", 1.0, NULL);

    g_object_set(G_OBJECT(player->mp3source->filesrc), "location",
        "/home/rugvip/music/Grendel/Best Of Grendel/Harsh Generation", NULL);

    gst_element_set_state(player->pipeline, GST_STATE_PLAYING);
}

void player_start(Player *player)
{
    g_print("Running...\n");
    g_main_loop_run(player->main_loop);

    g_print("Returned, stopping playback\n");
    gst_element_set_state(player->pipeline, GST_STATE_NULL);

    g_print("Deleting pipeline\n");
    gst_object_unref(GST_OBJECT(player->pipeline));
    g_source_remove(player->bus_watch_id);
    g_main_loop_unref(player->main_loop);
}
