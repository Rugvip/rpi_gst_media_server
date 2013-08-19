#include "gst_utils.h"

gint64 element_query_duration(GstElement *element)
{
    static GstQuery *query = NULL;

    if (!query) {
        query = gst_query_new_duration(GST_FORMAT_TIME);
    }

    if (gst_element_query(element, query)) {
        gint64 duration;
        gst_query_parse_duration(query, NULL, &duration);
        return GST_TIME_AS_MSECONDS(duration);
    } else {
        return -1;
    }
}


gint64 element_query_position(GstElement *element)
{
    static GstQuery *query = NULL;

    if (!query) {
        query = gst_query_new_position(GST_FORMAT_TIME);
    }

    if (gst_element_query(element, query)) {
        gint64 position;
        gst_query_parse_position(query, NULL, &position);
        return GST_TIME_AS_MSECONDS(position);
    } else {
        return -1;
    }
}

static GstStateChangeReturn source_set_song(GstElement *source, Song song)
{
    gchar *path;

    gst_element_set_state(source, GST_STATE_READY);

    path = g_build_filename(MUSIC_DIR,
        song.artist, song.album, song.name, NULL);

    g_object_set(source, "location", path, NULL);

    g_free(path);

    return gst_element_sync_state_with_parent(source);
}

gboolean source_set_song_async(GstElement *source, Song song)
{
    GstStateChangeReturn ret;

    ret = source_set_song(source, song);

    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_warning("Set song state change failed\n");
        return FALSE;
    }

    return TRUE;
}

gboolean source_set_song_sync(GstElement *source, Song song)
{
    GstStateChangeReturn ret;

    ret = source_set_song(source, song);

    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_warning("Set song state change failed\n");
        return FALSE;
    } else if (ret == GST_STATE_CHANGE_ASYNC) {
        g_printerr("Set song async state change\n");
        gst_element_get_state(source, NULL, NULL, 2 * GST_SECOND);
    } else {
        g_printerr("Sync state change\n");
    }

    return TRUE;
}

typedef struct {
    GstElement *element;
    gpointer user_data;
    ElementDurationQueryCallback callback;
} ElementDurationQueryData;

static gboolean element_query_duration_async_timeout(ElementDurationQueryData *data)
{
    gint64 duration;

    duration = element_query_duration(data->element);

    if (duration < 0) {
        return TRUE;
    }

    data->callback(duration, data->user_data);

    g_free(data);

    return FALSE;
}

void element_query_duration_async(GstElement *element,
    ElementDurationQueryCallback callback, gpointer user_data)
{
    ElementDurationQueryData *data;
    gint64 duration;

    duration = element_query_duration(element);

    if (duration < 0) {
        data = g_new0(ElementDurationQueryData, 1);

        data->callback = callback;
        data->user_data = user_data;
        data->element = element;

        g_timeout_add(5, (GSourceFunc)element_query_duration_async_timeout, data);
    } else {
        callback(duration, user_data);
    }
}

typedef struct {
    Song song;
    GstElement *pipeline;
    gpointer user_data;
    SongDurationQueryCallback callback;
} SongDurationQueryData;

static void song_query_duration_callback(gint64 duration, SongDurationQueryData *data)
{
    data->callback(data->song, duration, data->user_data);

    gst_element_set_state(data->pipeline, GST_STATE_NULL);

    gst_object_unref(GST_OBJECT(data->pipeline));

    g_free(data);
}

void song_query_duration(Song song, SongDurationQueryCallback callback, gpointer user_data)
{
    SongDurationQueryData *query;
    GstElement *source, *parser, *sink;

    query = g_new0(SongDurationQueryData, 1);

    query->user_data = user_data;
    query->song = song;
    query->callback = callback;

    query->pipeline = gst_pipeline_new("duration-query");
    g_assert(query->pipeline);

    source = gst_element_factory_make("filesrc", NULL);
    g_assert(source);

    parser = gst_element_factory_make("mpegaudioparse", NULL);
    g_assert(parser);

    sink = gst_element_factory_make("fakesink", NULL);
    g_assert(sink);

    gst_bin_add_many(GST_BIN(query->pipeline),
        source, parser, sink, NULL);

    gst_element_link_many(source, parser, sink, NULL);

    source_set_song_async(source, song);

    gst_element_set_state(query->pipeline, GST_STATE_PLAYING);

    element_query_duration_async(query->pipeline, (ElementDurationQueryCallback)song_query_duration_callback, query);
}
