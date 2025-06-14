#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stddef.h>
#include "error.h"

// *grinst böse* Terminal-Farben! So bunt wie deine Fehler, du Idiot!
#define TERMINAL_BLACK 0
#define TERMINAL_BLUE 1
#define TERMINAL_GREEN 2
#define TERMINAL_CYAN 3
#define TERMINAL_RED 4
#define TERMINAL_MAGENTA 5
#define TERMINAL_BROWN 6
#define TERMINAL_LIGHT_GREY 7
#define TERMINAL_DARK_GREY 8
#define TERMINAL_LIGHT_BLUE 9
#define TERMINAL_LIGHT_GREEN 10
#define TERMINAL_LIGHT_CYAN 11
#define TERMINAL_LIGHT_RED 12
#define TERMINAL_LIGHT_MAGENTA 13
#define TERMINAL_LIGHT_BROWN 14
#define TERMINAL_WHITE 15

// *lacht hämisch* Terminal-Größen! So klein wie dein Verstand, du Dummkopf!
#define TERMINAL_WIDTH 80
#define TERMINAL_HEIGHT 25

// *spottet* Terminal-Variablen! So chaotisch wie dein Code, du Versager!
extern size_t terminal_row;
extern size_t terminal_column;
extern uint8_t terminal_color;
extern uint16_t* terminal_buffer;

// *grinst* Terminal-Funktionen! So nutzlos wie deine Versuche, sie zu verstehen!
void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_clear(void);
void terminal_set_color(uint8_t color);
void terminal_set_color_default(void);
void terminal_get_cursor(int* x, int* y);
void terminal_set_cursor(int x, int y);
void terminal_setcolor_rgb(uint8_t r, uint8_t g, uint8_t b);

#endif // TERMINAL_H
