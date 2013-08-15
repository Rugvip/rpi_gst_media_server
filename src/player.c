#include "player.h"
#include "gst_utils.h"

#include <math.h>

gint64 player_get_duration(Player *player)
{
    return element_query_duration(player->source[player->source_id]->parser);
}

gint64 player_get_position(Player *player)
{
    return element_query_position(player->source[player->source_id]->parser);
}

void player_seek(Player *player, gint64 ms)
{
    gboolean ret;
    g_printerr("Seek to %ld\n", ms);
    ret = gst_element_seek_simple(player->source[0]->decoder,
                                  GST_FORMAT_TIME,
                                  GST_SEEK_FLAG_ACCURATE,
                                  ms * GST_MSECOND);
    if(ret) {
        g_printerr("seek done\n");
    } else {
        g_warning("seek failed\n");
    }
}

void player_pause(Player *player)
{
    gst_element_set_state(player->pipeline, GST_STATE_PAUSED);
}

static void player_set_song_callback(gint64 duration, Server *server)
{
    gint64 position;

    position = player_get_position(server->player);
    position = position < 0 ? 0 : position;

    ResponsePlaying response = {
        .song = server->player->source[0]->song,
        .duration = duration,
        .position = position,
    };

    jsonio_broadcast_packet(server, jsonio_response_playing_packet(&response));
}

gboolean player_set_song(Player *player, Song song)
{
    gst_element_set_state(player->source[player->source_id]->bin, GST_STATE_READY);

    if (!source_set_song_sync(player->source[player->source_id]->filesrc, song)) {
        return FALSE;
    }

    element_query_duration_async(player->source[player->source_id]->parser,
        (ElementDurationQueryCallback)player_set_song_callback, player->server);

    return TRUE;
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, Player *player)
{
    UNUSED(bus);
    // g_printerr("Bus call %s\n", gst_message_type_get_name(GST_MESSAGE_TYPE(msg)));
    gint64 t = 0;
    GstFormat fmt;
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_TAG: {
        GstTagList *tags = NULL;

        gst_message_parse_tag(msg, &tags);
        // g_printerr("Got tags from element %s\n", GST_OBJECT_NAME(msg->src));
        // g_printerr("Tags: %s\n", gst_tag_list_to_string(tags));
        gst_tag_list_free(tags);
        break;
    }
    case GST_MESSAGE_DURATION_CHANGED:
        gst_message_parse_duration(msg, &fmt, &t);
        g_printerr("New duration: %ld\n", t);
        break;
    case GST_MESSAGE_SEGMENT_DONE:
        // gst_element_seek(player->source[0]->decoder, 1.0, GST_FORMAT_TIME,
        //     GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SEGMENT | GST_SEEK_FLAG_ACCURATE,
        //     GST_SEEK_TYPE_SET, 22000 * GST_MSECOND, GST_SEEK_TYPE_SET, (30000) * GST_MSECOND);
        g_printerr("Segment done\n");
        break;
    case GST_MESSAGE_EOS:
        g_printerr("End of stream\n");
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

    player->source_id = 0;

    return player;
}

static void mp3_source_init(MP3Source *src, const gchar *name)
{
    src->filesrc = gst_element_factory_make("filesrc"        , "file-src");
    g_assert(src->filesrc);

    src->parser  = gst_element_factory_make("mpegaudioparse" , "mpeg-audio-parser");
    g_assert(src->parser);

    src->decoder = gst_element_factory_make("mpg123audiodec" , "mpeg-audio-decoder");
    g_assert(src->decoder);

    src->bin = gst_bin_new(name);
    g_assert(src->bin);

    gst_bin_add_many(GST_BIN(src->bin), src->filesrc, src->parser, src->decoder, NULL);
    gst_element_link_many(src->filesrc, src->parser, src->decoder, NULL);

    src->pad = gst_element_get_static_pad(src->decoder, "src");
    gst_element_add_pad(src->bin, gst_ghost_pad_new("src", src->pad));
    gst_object_unref(GST_OBJECT(src->pad));

    gst_object_ref(src->bin);
}

static GstPadProbeReturn pad_probe_cb(GstPad *pad, GstPadProbeInfo *info, Player *player)
{
    UNUSED(pad);
    UNUSED(info);
    // gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));
    gst_pad_send_event(player->source[0]->adderPad, gst_event_new_eos());

    return GST_PAD_PROBE_OK;
}

gdouble vol = -1.0;
void player_init(Player *player)
{
    GstBus *bus;

    g_setenv("GST_DEBUG_DUMP_DOT_DIR", "/home/rugvip/", TRUE);


    char *args[] = {
        "mp3net",
        "--gst-debug-no-color",
        "--gst-debug-level=2"
    };
    char **argv = args;
    int argc = sizeof(args)/sizeof(gchar *);

    gst_init(&argc, &argv);

    player->main_loop = g_main_loop_new(NULL, FALSE);

    player->pipeline = gst_pipeline_new("mediaplayer");
    g_assert(player->pipeline);

    player->adder = gst_element_factory_make("adder" , "adder");
    g_assert(player->adder);

    player->volume = gst_element_factory_make("volume" , "volume");
    g_assert(player->volume);

    player->equalizer = gst_element_factory_make("equalizer-10bands" , "equalizer");
    g_assert(player->equalizer);

    player->sink = gst_element_factory_make("alsasink" , "alsa-sink");
    g_assert(player->sink);

    mp3_source_init(player->source[0], "mp3-source-0");
    mp3_source_init(player->source[1], "mp3-source-1");

    gst_bin_add(GST_BIN(player->pipeline), player->source[0]->bin);
    gst_bin_add(GST_BIN(player->pipeline), player->source[1]->bin);

    gst_bin_add_many(GST_BIN(player->pipeline),
        player->adder, player->volume, player->equalizer, player->sink, NULL);

    gst_element_link(player->source[0]->bin, player->adder);
    gst_element_link(player->source[1]->bin, player->adder);

    gst_element_link_many(player->adder, player->volume, player->equalizer, player->sink, NULL);

    player->source[0]->adderPad = gst_element_get_static_pad(player->adder, "sink_0");
    g_assert(player->source[0]->adderPad);

    player->source[1]->adderPad = gst_element_get_static_pad(player->adder, "sink_1");
    g_assert(player->source[1]->adderPad);

    bus = gst_pipeline_get_bus(GST_PIPELINE(player->pipeline));
    player->bus_watch_id = gst_bus_add_watch(bus, (GstBusFunc) bus_call, player);
    gst_object_unref(bus);

    gboolean xfade(Player *player)
    {
        // g_printerr("Hi %2.2f %2.2f %2.2f\n", vol, fabs(vol), 1.0 - fabs(vol));
        g_object_set(player->source[0]->adderPad, "volume", fabs(vol), NULL);
        g_object_set(player->source[1]->adderPad, "volume", 1.0 - fabs(vol), NULL);
        vol += 0.02;
        if (vol > 1.0) {
            gst_pad_add_probe(player->source[0]->pad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
                (GstPadProbeCallback) pad_probe_cb, player, NULL);
            // gst_element_set_state(player->source[0]->decoder, GST_STATE_READY);
            // gst_element_unlink(player->source[0]->decoder, player->adder);
            // gst_bin_remove(GST_BIN(player->pipeline), player->source[0]->bin);
            // gst_element_set_state(player->pipeline, GST_STATE_PLAYING);
            g_object_set(player->source[1]->adderPad, "volume", 0.5, NULL);
            vol = -1.0;
            return FALSE;
        }
        return TRUE;
    }

    // g_timeout_add(50, (GSourceFunc) xfade, player);

    g_object_set(player->sink, "sync", FALSE, NULL);

    g_object_set(player->source[0]->adderPad, "volume", 1.0, NULL);
    g_object_set(player->source[1]->adderPad, "volume", 1.0, NULL);

    g_object_set(player->volume, "volume", 1.0, NULL);

    g_object_set(player->equalizer, "band0", 4.0,  NULL);
    g_object_set(player->equalizer, "band1", 3.0,  NULL);
    g_object_set(player->equalizer, "band2", 2.0,  NULL);
    g_object_set(player->equalizer, "band3", 1.0,  NULL);

    source_set_song_async(player->source[0]->filesrc, (Song){"Youtube Mixes", "Industrial", "Dark Modulator V"});
    // source_set_song_sync(player->source[0]->filesrc, (Song){"Daft Punk", "Random Access Memories", "Get Lucky"});
    source_set_song_async(player->source[1]->filesrc, (Song){"Daft Punk", "Random Access Memories", "Contact"});

    gst_element_set_state(player->pipeline, GST_STATE_PAUSED);

    void cb(Song song, gint64 dur) {
        g_printerr("Song: %s, duration: %ld\n", song.name, dur);
    }

    song_query_duration((Song) {"Daft Punk", "Random Access Memories", "Touch"}, cb);

    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(player->pipeline), 0, "graph");
}

void player_start(Player *player)
{
    g_printerr("Running...\n");
    g_main_loop_run(player->main_loop);

    g_printerr("Returned, stopping playback\n");
    gst_element_set_state(player->pipeline, GST_STATE_NULL);

    g_printerr("Deleting pipeline\n");
    gst_object_unref(GST_OBJECT(player->pipeline));
    g_source_remove(player->bus_watch_id);
    g_main_loop_unref(player->main_loop);

    GOutputStream *out = g_unix_output_stream_new(3, TRUE);

    GError *err = NULL;

    // GFileIOStream fios = g_file_open_readwrite("fifo", NULL, &err);

    gssize s = g_output_stream_write(out, "Hello!", 7, NULL, &err);

    if (err) {
        g_printerr("Error: %s\n", err->message);
    } else {
        g_printerr("Written %ld\n", s);
    }
}
