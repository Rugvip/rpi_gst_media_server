#ifndef gst_utils_h
#define gst_utils_h

#include "common.h"

G_BEGIN_DECLS

typedef void (*SongDurationQueryCallback)(Song, gint64 duration, gpointer user_data);
typedef void (*ElementDurationQueryCallback)(gint64 duration, gpointer user_data);
typedef void (*TransitionDoneCallback)(gpointer user_data);
typedef gboolean (*TransitionTickCallback)(gfloat progress, gpointer user_data);

gint64 element_query_duration(GstElement *);
gint64 element_query_position(GstElement *);
gboolean source_set_song_async(GstElement *, Song);
gboolean source_set_song_sync(GstElement *, Song);
void element_query_duration_async(GstElement *, ElementDurationQueryCallback, gpointer user_data);
void song_query_duration(Song, SongDurationQueryCallback, gpointer user_data);

void do_transition(guint duration, TransitionTickCallback, TransitionDoneCallback, gpointer user_data);

void remove_bin_from_pipeline(GstBin *, GstPipeline *, GstPad *);

G_END_DECLS

#endif /* gst_utils_h */
