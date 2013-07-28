#ifndef jsongen_h
#define jsongen_h

#include "pipeline.h"

typedef struct {
    void *priv;
} JsonPacket;

JsonPacket *jsongen_playing(Song song, gint duration, gint position);

JsonPacket *jsongen_paused(gint position);

JsonPacket *jsongen_eq(gint gain[NUM_EQ_BANDS]);

JsonPacket *jsongen_volume(gint volume);

JsonPacket *jsongen_info(Song song, gint duration, gint position);

#endif /* jsongen_h */
