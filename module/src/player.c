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

static void player_set_song_callback(gint64 duration, Player *player)
{
    gint64 position;

    position = player_get_position(player);
    position = position < 0 ? 0 : position;

    UNUSED(duration);
    // TODO: Playing callback
}

gboolean player_set_song(Player *player, Song song)
{
    gst_element_set_state(player->source[player->source_id]->bin, GST_STATE_READY);

    if (!source_set_song_sync(player->source[player->source_id]->filesrc, song)) {
        return FALSE;
    }

    element_query_duration_async(player->source[player->source_id]->parser,
        (ElementDurationQueryCallback)player_set_song_callback, player);

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
        // gst_element_set_state(player->pipeline, GST_STATE_PAUSED);
        break;

    case GST_MESSAGE_ERROR: {
            gchar *debug;
            GError *error;

            gst_message_parse_error(msg, &error, &debug);
            g_free(debug);

            g_printerr("Error: %s\n", error->message);
            g_error_free(error);
            exit(0);
            break;
        }
    default:
        break;
    }

    return TRUE;
}

static MP3Source *mp3_source_alloc()
{
    MP3Source *source;

    source = g_new0(MP3Source, 1);

    return source;
}

Player *player_alloc()
{
    Player *player;

    player = g_new0(Player, 1);

    player->source[0] = mp3_source_alloc();
    player->source[1] = mp3_source_alloc();

    player->source_id = 0;

    return player;
}

static MP3Source *mp3_source_init(MP3Source *src, const gchar *name)
{
    src->filesrc = gst_element_factory_make("filesrc", "file-src");
    g_assert(src->filesrc);

    src->parser  = gst_element_factory_make("mpegaudioparse", "mpeg-audio-parser");
    g_assert(src->parser);

    src->decoder = gst_element_factory_make("mpg123audiodec", "mpeg-audio-decoder");
    g_assert(src->decoder);

    src->valve = gst_element_factory_make("valve", "valve");
    g_assert(src->valve);

    src->bin = gst_bin_new(name);
    g_assert(src->bin);

    gst_bin_add_many(GST_BIN(src->bin), src->filesrc, src->parser, src->decoder, src->valve, NULL);
    gst_element_link_many(src->filesrc, src->parser, src->decoder, src->valve, NULL);

    src->ghost_pad = gst_element_get_static_pad(src->valve, "src");
    gst_element_add_pad(src->bin, gst_ghost_pad_new("src", src->ghost_pad));
    gst_object_unref(GST_OBJECT(src->ghost_pad));

    src->adder_pad = NULL;

    gst_object_ref(src->bin);

    return src;
}

void player_link_source(Player *player, MP3Source *src)
{
    GstPadLinkReturn lret;
    gboolean success;
    gchar *name;

    gst_bin_add(GST_BIN(player->pipeline), src->bin);

    g_object_set(src->valve, "drop", FALSE, NULL);

    g_assert(src->ghost_pad);
    g_assert(!src->adder_pad);

    src->adder_pad = gst_element_get_request_pad(player->adder, "sink_%u");
    g_assert(src->adder_pad);

    name = gst_pad_get_name(src->adder_pad);
    lret = gst_element_link_pads(src->bin, NULL, player->adder, name);
    g_free(name);

    if (GST_PAD_LINK_FAILED(lret)) {
        g_print("player link failed: %d\n", lret);
        g_assert_not_reached();
    }

    success = gst_element_sync_state_with_parent(GST_ELEMENT(src->bin));
    g_assert(success);

    gboolean ret;
    ret = gst_element_seek_simple(src->decoder,
                                  GST_FORMAT_TIME,
                                  GST_SEEK_FLAG_ACCURATE,
                                  50000 * GST_MSECOND);
    if(ret) {
        g_printerr("seek done\n");
    } else {
        g_warning("seek failed\n");
    }
}

void player_unlink_source(Player *player, MP3Source *src)
{
    gboolean success;

    g_assert(src->adder_pad);
    g_assert(src->ghost_pad);

    g_object_set(src->valve, "drop", TRUE, NULL);

    GstPad *pad;
    pad = gst_pad_get_peer(src->adder_pad);
    success = gst_pad_unlink(pad, src->adder_pad);
    g_assert(success);
    gst_object_unref(pad);

    gst_element_set_state(src->bin, GST_STATE_NULL);
    gst_bin_remove(GST_BIN(player->pipeline), src->bin);

    gst_element_release_request_pad(player->adder, src->adder_pad);
    src->adder_pad = NULL;
}

Player *player_init(Player *player)
{
    GstBus *bus;

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

    gst_bin_add_many(GST_BIN(player->pipeline),
        player->adder, player->volume, player->equalizer, player->sink, NULL);

    mp3_source_init(player->source[0], "mp3-source-0");
    mp3_source_init(player->source[1], "mp3-source-1");

    player_link_source(player, player->source[0]);
    // player_link_source(player, player->source[1]);

    gst_element_link_many(player->adder, player->volume, player->equalizer, player->sink, NULL);

    bus = gst_pipeline_get_bus(GST_PIPELINE(player->pipeline));
    player->bus_watch_id = gst_bus_add_watch(bus, (GstBusFunc) bus_call, player);
    gst_object_unref(bus);

    // gboolean xfade(Player *player)
    // {
    //     // g_printerr("Hi %2.2f %2.2f %2.2f\n", vol, fabs(vol), 1.0 - fabs(vol));
    //     g_object_set(player->source[0]->adder_pad, "volume", fabs(vol), NULL);
    //     g_object_set(player->source[1]->adder_pad, "volume", 1.0 - fabs(vol), NULL);
    //     vol += 0.02;
    //     if (vol > 1.0) {
    //         gst_pad_add_probe(player->source[0]->pad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
    //             (GstPadProbeCallback) pad_probe_cb, player, NULL);
    //         // gst_element_set_state(player->source[0]->decoder, GST_STATE_READY);
    //         // gst_element_unlink(player->source[0]->decoder, player->adder);
    //         // gst_bin_remove(GST_BIN(player->pipeline), player->source[0]->bin);
    //         // gst_element_set_state(player->pipeline, GST_STATE_PLAYING);
    //         g_object_set(player->source[1]->adder_pad, "volume", 0.5, NULL);
    //         vol = -1.0;
    //         return FALSE;
    //     }
    //     return TRUE;
    // }

    // g_timeout_add(50, (GSourceFunc) xfade, player);

    g_object_set(player->sink, "sync", FALSE, NULL);

    // g_object_set(player->source[0]->adder_pad, "volume", 1.0, NULL);
    // g_object_set(player->source[1]->adder_pad, "volume", 1.0, NULL);

    g_object_set(player->volume, "volume", 1.0, NULL);

    g_object_set(player->equalizer, "band0", 4.0,  NULL);
    g_object_set(player->equalizer, "band1", 3.0,  NULL);
    g_object_set(player->equalizer, "band2", 2.0,  NULL);
    g_object_set(player->equalizer, "band3", 1.0,  NULL);

    return player;
}

GstPadProbeReturn cbcb(GstPad *pad, GstPadProbeInfo *info, Player *player) {
    g_print("wat\n");
    // g_object_set(player->source[1]->output,
    //     "active-pad", player->source[1]->play_pad, NULL);
    // gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));
    // peer = gst_pad_get_peer(player->source[1]->pad);
    // gst_object_unref(peer);
    // g_assert(peer);
    // gst_element_unlink(player->source[1]->bin, player->adder);
    // gst_element_release_request_pad(player->adder, peer);
    // gst_element_set_state(GST_ELEMENT(player->source[1]->bin), GST_STATE_READY);
    return GST_PAD_PROBE_OK;
}

gboolean watwat(Player *player)
{
    g_print("unlink\n");
    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(player->pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "uno");
    // player_unlink_source(player, player->source[0]);
    // player_unlink_source(player, player->source[1]);
    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(player->pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "dos");
    g_print("done\n");
    // gst_pad_add_probe(player->source[1]->ghost_pad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
    //     (GstPadProbeCallback) cbcb, player, NULL);

    // g_object_set(player->source[1]->output,
    //     "active-pad", player->source[1]->fake_pad, NULL);
    return FALSE;
}

gboolean derp(Player *player)
{
    static gint id = 1;
    static gint op = 1;
    if (op) {
        g_print("link %d\n", id);
        player_link_source(player, player->source[id]);
        id = (id + 1) % 2;
        op = (op + 1) % 2;
    } else {
        g_print("unlink %d\n", id);
        player_unlink_source(player, player->source[id]);
        op = (op + 1) % 2;
    }
    return TRUE;
}

void player_start(Player *player)
{
    g_printerr("starting...\n");

    source_set_song_async(player->source[0]->filesrc, (Song){"Korsakoff", "Stiletto", "Center of mass"});
    // source_set_song_sync(player->source[0]->filesrc, (Song){"Daft Punk", "Random Access Memories", "Get Lucky"});
    source_set_song_async(player->source[1]->filesrc, (Song){"Korsakoff", "Stiletto", "Section Break"});

    gst_element_set_state(player->pipeline, GST_STATE_PLAYING);
    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(player->pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "wat");
    // gst_element_seek()

    // g_timeout_add(1000, (GSourceFunc) watwat, player);
    g_timeout_add(2000, (GSourceFunc) derp, player);
}

void player_stop(Player *player)
{
    g_printerr("\n\n\nSTAHP\n\n\nSTAHP...stopping\n");
    gst_element_set_state(player->pipeline, GST_STATE_NULL);

    gst_object_unref(GST_OBJECT(player->pipeline));
    g_source_remove(player->bus_watch_id);
}
