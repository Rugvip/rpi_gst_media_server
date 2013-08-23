#ifndef pipeline_h
#define pipeline_h

#include "common.h"

G_BEGIN_DECLS

gint64 player_get_duration(Player *);
gint64 player_get_position(Player *);
void player_seek(Player *, gint64 mstime);
void player_pause(Player *);
gboolean player_set_song(Player *, Song song);

Player *player_alloc();
Player *player_init(Player *, int argc, char **argv);
void player_start(Player *);
void player_stop(Player *);

G_END_DECLS

#endif /* pipeline_h */
