#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>
#include <stdint.h>

extern size_t terminal_row;
extern size_t terminal_column;
extern uint8_t terminal_color;
extern uint16_t* terminal_buffer;

void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
uint8_t terminal_get_color(void);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_putchar(char c);
void terminal_writestring(const char* data);
void terminal_write(const char* data, size_t size);
void terminal_clear(void);
void terminal_set_cursor(int x, int y);
void terminal_get_cursor(int* x, int* y);
void terminal_setcolor_rgb(uint8_t r, uint8_t g, uint8_t b);
void terminal_set_color_default(void);

// Wrapper für Kompatibilität
static inline void terminal_set_color(uint8_t color) {
    terminal_setcolor(color);
}

#endif // TERMINAL_H
