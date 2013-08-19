#ifndef jsonio_h
#define jsonio_h

#include "common.h"

G_BEGIN_DECLS

void jsonio_read(Player *);
void jsonio_write(gpointer output);

void jsonio_set_input_handler(Player *, InputType, InputHandler);

G_END_DECLS

#endif /* jsonio_h */
