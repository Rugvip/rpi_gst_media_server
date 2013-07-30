#ifndef jsonio_h
#define jsonio_h

#include "common.h"
#include "jsonparse.h"
#include "jsongen.h"

void jsonio_read_request(Client *);
void jsonio_set_request_handler(Server *, RequestType, RequestHandler);

void jsonio_send_packet(Client *, JsonPacket *);
void jsonio_broadcast_packet(Server *, JsonPacket *);

#endif /* jsonio_h */
