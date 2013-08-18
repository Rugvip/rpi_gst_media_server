#ifndef jsonio_h
#define jsonio_h

#include "common.h"

void jsonio_read(Player *);
void jsonio_write(Player *, gpointer output);

void jsonio_set_input_handler(Player *, InputType, InputHandler);

#endif /* jsonio_h */
