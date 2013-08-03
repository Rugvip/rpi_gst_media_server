#ifndef jsonio_h
#define jsonio_h

#include "common.h"

JsonPacket *jsonio_response_playing_packet(ResponsePlaying *response);
JsonPacket *jsonio_response_paused_packet(ResponsePaused *response);
JsonPacket *jsonio_response_eq_packet(ResponseEq *response);
JsonPacket *jsonio_response_volume_packet(ResponseVolume *response);
JsonPacket *jsonio_response_info_packet(ResponseInfo *response);

void jsonio_read_request(Client *);
void jsonio_set_request_handler(Server *, RequestType, RequestHandler);

void jsonio_send_packet(Client *, JsonPacket *);
void jsonio_broadcast_packet(Server *, JsonPacket *);

#endif /* jsonio_h */
