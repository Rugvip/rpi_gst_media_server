#ifndef handler_h
#define handler_h

#include "common.h"

void handle_info_request(RequestInfo *);
void handle_play_request(RequestPlay *);
void handle_pause_request(RequestPause *);
void handle_next_request(RequestNext *);
void handle_seek_request(RequestSeek *);
void handle_volume_request(RequestVolume *);
void handle_eq_request(RequestEq *);

#endif /* handler_h */
