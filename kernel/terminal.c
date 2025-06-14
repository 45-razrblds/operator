#include <stdint.h>
#include <stddef.h>
#include "terminal.h"
#include "io.h"
#include "error.h"
#include "minilibc.h"

// VGA-Buffer
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

// Terminal-Position
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

// Terminal-Initialisierung
void terminal_initialize(void) {
    terminal_buffer = (uint16_t*)0xB8000;
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = TERMINAL_LIGHT_GREY;
    terminal_clear();
}

// Terminal-Farbe setzen
void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

// Terminal-Farbe abrufen
uint8_t terminal_get_color(void) {
    return terminal_color;
}

// Terminal-Zeichen ausgeben
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * TERMINAL_WIDTH + x;
    terminal_buffer[index] = (uint16_t)c | (uint16_t)color << 8;
}

// Terminal-Zeichen ausgeben
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row == TERMINAL_HEIGHT) {
            terminal_row = 0;
        }
        return;
    }
    
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == TERMINAL_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == TERMINAL_HEIGHT) {
            terminal_row = 0;
        }
    }
}

// Terminal-String ausgeben
void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

// Terminal-Daten ausgeben
void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

// Terminal löschen
void terminal_clear(void) {
    for (size_t y = 0; y < TERMINAL_HEIGHT; y++) {
        for (size_t x = 0; x < TERMINAL_WIDTH; x++) {
            terminal_putentryat(' ', terminal_color, x, y);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

// Cursor-Position setzen
void terminal_set_cursor(int x, int y) {
    if (x >= 0 && x < TERMINAL_WIDTH && y >= 0 && y < TERMINAL_HEIGHT) {
        terminal_column = x;
        terminal_row = y;
    }
}

// Cursor-Position abrufen
void terminal_get_cursor(int* x, int* y) {
    if (x) *x = terminal_column;
    if (y) *y = terminal_row;
}

// RGB-Farbe setzen
void terminal_setcolor_rgb(uint8_t r, uint8_t g, uint8_t b) {
    // Einfache RGB-zu-VGA-Farbkonvertierung
    uint8_t color = 0;
    if (r > 128) color |= 0x04; // Rot
    if (g > 128) color |= 0x02; // Grün
    if (b > 128) color |= 0x01; // Blau
    if (r > 128 || g > 128 || b > 128) color |= 0x08; // Hell
    terminal_setcolor(color);
}

// Standardfarbe setzen
void terminal_set_color_default(void) {
    terminal_setcolor(TERMINAL_LIGHT_GREY);
}
