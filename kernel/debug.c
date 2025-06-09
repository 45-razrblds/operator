#include "debug.h"
#include <stdint.h>
#include "terminal.h"
#include "memory.h"
#include "minilibc.h"
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
int debug_mode = 0;
static uint16_t debug_row_start = VGA_HEIGHT - 3;
extern uint16_t* terminal_buffer;
extern uint16_t terminal_row;
extern uint16_t terminal_column;
extern uint8_t terminal_color;
void draw_debug_info() {
    if (!debug_mode) return;
    uint16_t old_row = terminal_row;
    uint16_t old_col = terminal_column;
    uint8_t old_color = terminal_color;
    if (terminal_row >= debug_row_start) {
        terminal_row = debug_row_start - 1;
        terminal_column = 0;
    }
    for (int i = 0; i < VGA_WIDTH; i++) {
        terminal_buffer[debug_row_start * VGA_WIDTH + i] = (0x07 << 8) | '-';
    }
    for (int i = debug_row_start + 1; i < VGA_HEIGHT; i++) {
        for (int j = 0; j < VGA_WIDTH; j++) {
            terminal_buffer[i * VGA_WIDTH + j] = (0x07 << 8) | ' ';
        }
    }
    size_t used = get_heap_used();
    size_t total = get_heap_total();
    size_t percent = (used * 100) / total;
    char buf[64];
    snprintf(buf, sizeof(buf), "Memory Usage: %d/%d KB (%d%%)", (int)(used/1024), (int)(total/1024), (int)percent);
    int pos = 0;
    int debug_line = debug_row_start + 1;
    for (int i = 0; buf[i] && pos < VGA_WIDTH; i++) {
        terminal_buffer[debug_line * VGA_WIDTH + pos++] = (0x0B << 8) | buf[i];
    }
    pos += 2;
    int bar_width = VGA_WIDTH - pos - 2;
    if (bar_width > 40) bar_width = 40;
    terminal_buffer[debug_line * VGA_WIDTH + pos++] = (0x07 << 8) | '[';
    int filled = (bar_width * percent) / 100;
    uint8_t bar_color = percent > 90 ? 0x0C : percent > 70 ? 0x0E : 0x0A;
    for (int i = 0; i < filled && pos < VGA_WIDTH - 1; i++) {
        terminal_buffer[debug_line * VGA_WIDTH + pos++] = (bar_color << 8) | '|';
    }
    for (int i = filled; i < bar_width && pos < VGA_WIDTH - 1; i++) {
        terminal_buffer[debug_line * VGA_WIDTH + pos++] = (0x08 << 8) | '.';
    }
    terminal_buffer[debug_line * VGA_WIDTH + pos++] = (0x07 << 8) | ']';
    terminal_row = old_row >= debug_row_start ? debug_row_start - 1 : old_row;
    terminal_column = old_row >= debug_row_start ? 0 : old_col;
    terminal_color = old_color;
}
void toggle_debug_mode() {
    debug_mode = !debug_mode;
    if (debug_mode) {
        debug_row_start = VGA_HEIGHT - 3;
        terminal_writestring("Debug mode enabled.\n");
    } else {
        terminal_writestring("Debug mode disabled.\n");
    }
}
