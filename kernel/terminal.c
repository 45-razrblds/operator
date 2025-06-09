#include "terminal.h"
#include <stdint.h>
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000
uint16_t* terminal_buffer;
uint16_t terminal_row;
uint16_t terminal_column;
uint8_t terminal_color;
void terminal_initialize() {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = 0x07;
    terminal_buffer = (uint16_t*) VGA_MEMORY;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        terminal_buffer[i] = (terminal_color << 8) | ' ';
    }
}
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            for (int y = 0; y < VGA_HEIGHT - 1; y++) {
                for (int x = 0; x < VGA_WIDTH; x++) {
                    terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
                }
            }
            for (int x = 0; x < VGA_WIDTH; x++) {
                terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (terminal_color << 8) | ' ';
            }
            terminal_row--;
        }
        return;
    }
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] = (terminal_color << 8) | ' ';
        }
        return;
    }
    terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] = (terminal_color << 8) | c;
    terminal_column++;
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }
}
void terminal_writestring(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i]);
    }
}
void terminal_set_color(uint8_t color) {
    terminal_color = color;
}
