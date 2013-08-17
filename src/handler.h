#ifndef handler_h
#define handler_h

#include "common.h"

void handler_info(InputInfo *);
void handler_play(InputPlay *);
void handler_pause(InputPause *);
void handler_next(InputNext *);
void handler_seek(InputSeek *);
void handler_volume(InputVolume *);
void handler_eq(InputEq *);

#endif /* handler_h */
