#ifndef TERMINAL_H
#define TERMINAL_H
#include <stdint.h>
extern uint16_t* terminal_buffer;
extern uint16_t terminal_row;
extern uint16_t terminal_column;
extern uint8_t terminal_color;
void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_writestring(const char* str);
void terminal_set_color(uint8_t color);
void terminal_setcolor_rgb(uint8_t r, uint8_t g, uint8_t b);
void terminal_setcolor_default(void);
#endif // TERMINAL_H
