#ifndef jsongen_h
#define jsongen_h

#include "common.h"

void jsongen_free_packet(JsonPacket *);
void jsongen_write_packet(GOutputStream *, JsonPacket *);

JsonPacket *jsongen_playing(Song song, gint duration, gint position);
JsonPacket *jsongen_paused(gint position);
JsonPacket *jsongen_eq(gdouble gain[NUM_EQ_BANDS]);
JsonPacket *jsongen_volume(gdouble volume);
JsonPacket *jsongen_info(Song song, gint duration, gint position);

#endif /* jsongen_h */
