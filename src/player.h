#ifndef pipeline_h
#define pipeline_h

#include "common.h"
#include "jsonio.h"

gint64 player_get_duration(Player *);
gint64 player_get_position(Player *);
void player_set_segment(Player *, gint64 mstime);
void player_pause(Player *);
gboolean player_set_song(Player *, Song song);

Player *player_new(Server *);
void player_init(Player *);
void player_start(Player *);

#endif /* pipeline_h */
