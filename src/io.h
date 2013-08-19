#ifndef server_h
#define server_h

#include "common.h"

G_BEGIN_DECLS

void io_init(Player *player);
void io_start(Player *player);
void io_close(Player *player);

G_END_DECLS

#endif /* server_h */
