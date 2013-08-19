#ifndef jsongen_h
#define jsongen_h

#include "common.h"
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

void jsongen_playing(OutputPlaying *);
void jsongen_paused(OutputPaused *);
void jsongen_eq(OutputEq *);
void jsongen_volume(OutputVolume *);
void jsongen_info(OutputInfo *);
void jsongen_duration_result(OutputDurationResult *);

G_END_DECLS

#endif /* jsongen_h */
