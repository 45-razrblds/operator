#ifndef DEBUG_H
#define DEBUG_H

#include "error.h"

void debug_init(void);
void draw_debug_info(void);
void toggle_debug_mode(void);
extern int debug_mode;

#endif // DEBUG_H
