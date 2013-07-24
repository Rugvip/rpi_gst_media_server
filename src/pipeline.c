#include "pipeline.h"

#include <gst/gst.h>

GstElement *pipeline, *source, *parser, *decoder, *sink;

gint get_duration(){
    static GstQuery *query = NULL;

    if (!query) {
        query = gst_query_new_duration(GST_FORMAT_TIME);
    }

    if (gst_element_query(parser, query)) {
        gint64 duration;
        gst_query_parse_duration(query, NULL, &duration);
        return GST_TIME_AS_MSECONDS(duration);
    } else {
        return -1;
    }
}

gint get_position(){
    gint64 position = -1;
    if (gst_element_query_position(parser, GST_FORMAT_TIME, &position)) {
        return (gint) GST_TIME_AS_MSECONDS(position);
    } else {
        return -1;
    }
}

void set_position(gint ms)
{
    if(!gst_element_seek_simple(pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, ms * GST_MSECOND)) {
        g_warning("seek failed");
    } else {
        g_print("seek done.");
    }
}

gboolean timeout_duration_query(UserData *data) {
    gint duration = get_duration();
    if (duration < 0) {
        return TRUE;
    } else {
        data->value = duration;
        data->callback(data);
        return FALSE;
    }
}

gboolean play_file(const gchar *path, UserData *data)
{
    GstStateChangeReturn ret;
    gst_element_set_state(pipeline, GST_STATE_NULL);
    g_object_set(G_OBJECT(source), "location", path, NULL);
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_warning("State change failed");

    }
    g_timeout_add(100, (GSourceFunc)timeout_duration_query, data);
    g_print("Now playing: %s\n", path);
    return TRUE;
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *) data;

    UNUSED(loop);
    UNUSED(bus);

    switch (GST_MESSAGE_TYPE(msg)) {

    case GST_MESSAGE_EOS:
        g_print("End of stream\n");
        gst_element_set_state(pipeline, GST_STATE_PAUSED);
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

void start()
{
    GMainLoop *loop;

    GstBus *bus;
    guint bus_watch_id;


    char **argv = (char *[]) {
        "--gst-debug-level=3"
    };
    int argc = sizeof(argv)/sizeof(gchar *);

    gst_init(&argc, &argv);
    loop = g_main_loop_new(NULL, FALSE);

    /* Create gstreamer elements */
    pipeline = gst_pipeline_new ("mp3-player");
    source   = gst_element_factory_make ("filesrc"       , "file-source");
    parser   = gst_element_factory_make ("mpegaudioparse", "mpeg-parse");
    decoder  = gst_element_factory_make ("mad"           , "mad");
    sink     = gst_element_factory_make ("alsasink"      , "alsa-sink");

    g_assert(pipeline);
    g_assert(source);
    g_assert(parser);
    g_assert(decoder);
    g_assert(sink);

    const gchar *location = "/home/pi/music/Daft Punk/Random Access Memories/Contact";
    /* we set the input filename to the source element */
    g_object_set(G_OBJECT(source), "location", location, NULL);

    /* we add a message handler */
    bus = gst_pipeline_get_bus(GST_PIPELINE (pipeline));
    bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    /* we add all elements into the pipeline */
    gst_bin_add_many(GST_BIN(pipeline), source, parser, decoder, sink, NULL);

    /* we link the elements together */
    gst_element_link_many(source, parser, decoder, sink, NULL);

    /* Set the pipeline to "playing" state*/
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    /* Iterate */
    g_print("Running...\n");
    g_main_loop_run(loop);

    /* Out of the main loop, clean up nicely */
    g_print("Returned, stopping playback\n");

    g_print("Deleting pipeline\n");
    gst_object_unref(GST_OBJECT(pipeline));
    g_source_remove(bus_watch_id);
    g_main_loop_unref(loop);
}
