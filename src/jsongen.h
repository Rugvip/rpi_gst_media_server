#ifndef jsongen_h
#define jsongen_h

#include "common.h"
#include <json-glib/json-glib.h>

void jsongen_playing(GOutputStream *,Song song, gint duration, gint position);
void jsongen_paused(GOutputStream *,gint position);
void jsongen_eq(GOutputStream *,gdouble gain[NUM_EQ_BANDS]);
void jsongen_volume(GOutputStream *,gdouble volume);
void jsongen_info(GOutputStream *,Song song, gint duration, gint position);

#endif /* jsongen_h */
