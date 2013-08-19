#ifndef handler_h
#define handler_h

#include "common.h"

G_BEGIN_DECLS

void handler_handle_info(InputInfo *);
void handler_handle_play(InputPlay *);
void handler_handle_pause(InputPause *);
void handler_handle_next(InputNext *);
void handler_handle_seek(InputSeek *);
void handler_handle_volume(InputVolume *);
void handler_handle_eq(InputEq *);
void handler_handle_duration_query(InputDurationQuery *input);

G_END_DECLS

#endif /* handler_h */
