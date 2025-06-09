#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

// Special key codes (using values above normal ASCII range)
#define KEY_ESC    0x1B    // 27 (ASCII ESC)
#define KEY_UP     0x100
#define KEY_DOWN   0x101
#define KEY_LEFT   0x102
#define KEY_RIGHT  0x103
#define KEY_HOME   0x104
#define KEY_END    0x105
#define KEY_PGUP   0x106
#define KEY_PGDN   0x107
#define KEY_DEL    0x108

// Keyboard layout types
typedef enum {
    LAYOUT_QWERTY = 0,
    LAYOUT_QWERTZ = 1
} keyboard_layout_t;

// Function to set keyboard layout
void set_keyboard_layout(keyboard_layout_t layout);

// Function to get current layout name
const char* get_layout_name(void);

// Function to convert scancode to ASCII or special key code
uint16_t scancode_to_ascii(uint8_t scancode);

// Function to get the character corresponding to the last pressed key
uint16_t get_keyboard_char(void);

#endif // KEYBOARD_H