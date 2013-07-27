#ifndef jsongen_h
#define jsongen_h

#include "pipeline.h"

typedef struct {
    void *priv;
} JsonPacket;

JsonPacket *jsongen_playing(Client *)
JsonPacket *jsongen_paused(Client *)
JsonPacket *jsongen_time(Client *)
JsonPacket *jsongen_info(Client *)

void jsongen_send_packet(Client *, JsonPacket *);

#endif /* jsongen_h */
