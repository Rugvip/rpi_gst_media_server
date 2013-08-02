#include "player.h"

const gchar *const MUSIC_DIR = "/home/rugvip/music";

gint64 player_get_duration(Player *player)
{
    static GstQuery *query = NULL;

    if (!query) {
        query = gst_query_new_duration(GST_FORMAT_TIME);
    }

    if (gst_element_query(player->source[0]->parser, query)) {
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

    if (gst_element_query(player->source[0]->parser, query)) {
        gint64 position;
        gst_query_parse_position(query, NULL, &position);
        return GST_TIME_AS_MSECONDS(position);
    } else {
        return -1;
    }
}

void player_set_segment(Player *player, gint ms)
{
    gboolean ret;

    ret = gst_element_seek(player->source[0]->decoder, 1.0, GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SEGMENT,
        GST_SEEK_TYPE_SET, ms * GST_MSECOND,
        GST_SEEK_TYPE_SET, (16000 + 6000) * GST_MSECOND);
    if(ret) {
        g_print("seek done\n");
    } else {
        g_warning("seek failed\n");
    }
}

void player_pause(Player *player)
{
    gst_element_set_state(player->pipeline, GST_STATE_PAUSED);
}

static gboolean timeout_duration_query(Server *server)
{
    gint64 duration = player_get_duration(server->player);
    if (duration < 0) {
        return TRUE; /* Repeat timer */
    }

    JsonPacket *packet;
    gint64 position;

    position = player_get_position(server->player);

    position = position < 0 ? 0 : position;

    packet = jsongen_playing(server->player->source[0]->song, duration, position);
    jsonio_broadcast_packet(server, packet);

    return FALSE;
}

gboolean player_set_song(Player *player, Song song)
{
    GstStateChangeReturn ret;
    gst_element_set_state(player->pipeline, GST_STATE_READY); // FIXME: Needed?

    gchar *path;

    path = g_build_filename(MUSIC_DIR, song.artist, song.album, song.name, NULL);

    g_object_set(player->source[0]->filesrc, "location", path, NULL);
    ret = gst_element_set_state(player->pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_warning("State change failed");
        g_free(path);
        return FALSE;
    } else if (ret == GST_STATE_CHANGE_ASYNC) {
        g_print("Async\n");
        gst_element_get_state(player->pipeline, NULL, NULL, GST_SECOND*2);
    }
    if (timeout_duration_query(player->server)) {
        g_timeout_add(5, (GSourceFunc)timeout_duration_query, player->server);
    }
    g_print("Now playing: %s\n", path);

    g_free(path);

    return TRUE;
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, Player *player)
{
    UNUSED(bus);
    g_print("Bus call %s\n", gst_message_type_get_name(GST_MESSAGE_TYPE(msg)));
    gint64 t = 0;
    GstFormat fmt;
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_TAG: {
        GstTagList *tags = NULL;

        gst_message_parse_tag(msg, &tags);
        g_print("Got tags from element %s\n", GST_OBJECT_NAME(msg->src));
        g_print("Tags: %s\n", gst_tag_list_to_string(tags));
        gst_tag_list_free(tags);
        break;
    }
    case GST_MESSAGE_DURATION_CHANGED:
        gst_message_parse_duration(msg, &fmt, &t);
        g_print("New duration: %ld\n", t);
        break;
    case GST_MESSAGE_SEGMENT_DONE:
        gst_element_seek(player->source[0]->decoder, 1.0, GST_FORMAT_TIME,
            GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SEGMENT | GST_SEEK_FLAG_ACCURATE,
            GST_SEEK_TYPE_SET, 22000 * GST_MSECOND, GST_SEEK_TYPE_SET, (30000) * GST_MSECOND);
        g_print("Segment done\n");
        break;
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

    player->source[0] = mp3_source_new();
    player->source[1] = mp3_source_new();
    player->server = server;

    return player;
}

static void mp3_source_init(MP3Source *src, const gchar *name)
{
    GstPad *pad;

    src->filesrc = gst_element_factory_make("filesrc"        , "file-src");
    src->parser  = gst_element_factory_make("mpegaudioparse" , "mpeg-audio-parser");
    src->decoder = gst_element_factory_make("mpg123audiodec" , "mpeg-audio-decoder");
    src->volume  = gst_element_factory_make("volume" , "volume");

    g_assert(src->filesrc);
    g_assert(src->parser);
    g_assert(src->decoder);
    g_assert(src->volume);

    src->bin = gst_bin_new(name);
    g_assert(src->bin);

    gst_bin_add_many(GST_BIN(src->bin),
        src->filesrc, src->parser, src->decoder, src->volume, NULL);
    gst_element_link_many(src->filesrc, src->parser, src->decoder, src->volume, NULL);

    pad = gst_element_get_static_pad(src->volume, "src");
    gst_element_add_pad(src->bin, gst_ghost_pad_new("src", pad));
    gst_object_unref(GST_OBJECT(pad));
}

void player_init(Player *player)
{
    GstBus *bus;

    g_setenv("GST_DEBUG_DUMP_DOT_DIR", "/home/rugvip/", TRUE);


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
    player->adder = gst_element_factory_make("adder" , "adder");
    player->equalizer = gst_element_factory_make("equalizer-10bands" , "equalizer");
    player->sink      = gst_element_factory_make("alsasink" , "alsa-sink");

    g_assert(player->pipeline);
    g_assert(player->adder);
    g_assert(player->equalizer);
    g_assert(player->sink);

    mp3_source_init(player->source[0], "mp3-src-bin-left");
    mp3_source_init(player->source[1], "mp3-src-bin-right");

    gst_bin_add(GST_BIN(player->pipeline), player->source[0]->bin);
    gst_bin_add(GST_BIN(player->pipeline), player->source[1]->bin);

    gst_bin_add_many(GST_BIN(player->pipeline),
        player->adder, player->equalizer, player->sink, NULL);

    gst_element_link(player->source[0]->bin, player->adder);
    gst_element_link(player->source[1]->bin, player->adder);

    gst_element_link_many(player->adder, player->equalizer, player->sink, NULL);

    bus = gst_pipeline_get_bus(GST_PIPELINE(player->pipeline));
    player->bus_watch_id = gst_bus_add_watch(bus, (GstBusFunc) bus_call, player);
    gst_object_unref(bus);

    g_object_set(player->source[0]->volume, "volume", 0.7, NULL);
    g_object_set(player->source[1]->volume, "volume", 0.7, NULL);

    g_object_set(player->equalizer, "band0", 4.0,  NULL);
    g_object_set(player->equalizer, "band1", 3.0,  NULL);
    g_object_set(player->equalizer, "band2", 2.0,  NULL);
    g_object_set(player->equalizer, "band3", 1.0,  NULL);

    g_object_set(player->source[0]->filesrc, "location",
        "/home/rugvip/music/Grendel/Best Of Grendel/Harsh Generation", NULL);
    g_object_set(player->source[1]->filesrc, "location",
        "/home/rugvip/music/Grendel/Best Of Grendel/Void Malign", NULL);

    gst_element_set_state(player->pipeline, GST_STATE_PAUSED);

    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(player->pipeline), 0, "graph");
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
