#ifndef jsongen_h
#define jsongen_h

#include "pipeline.h"

#include <glib.h>

typedef struct {
    gint duration;
    gint position;
} JSON_PlaybackInfo;

void jsongen_playback_info(Client *client, JSON_PlaybackInfo *info);
void jsongen_playback_start(Client *client, JSON_PlaybackInfo *info);

#endif /* jsongen_h */
