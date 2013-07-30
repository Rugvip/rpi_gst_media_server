#ifndef pipeline_h
#define pipeline_h

#include "common.h"
#include "jsonio.h"

void handle_info_request(RequestInfo *);
void handle_play_request(RequestPlay *);
void handle_pause_request(RequestPause *);
void handle_next_request(RequestNext *);
void handle_seek_request(RequestSeek *);
void handle_volume_request(RequestVolume *);
void handle_eq_request(RequestEq *);

gint64 player_get_duration(Player *);
gint64 player_get_position(Player *);
void player_set_position(Player *, gint mstime);
void player_seek(Player *, gint ms);
gboolean player_set_song(Player *, Song song);

Player *player_new(Server *);
void player_init(Player *);
void player_start(Player *);

#endif /* pipeline_h */
