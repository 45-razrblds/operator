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

// Current keyboard layout
static keyboard_layout_t current_layout = LAYOUT_QWERTY;

// QWERTY layout scancode to ASCII mapping
static const uint16_t qwerty_layout[] = { 
    0, KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // Arrow keys (0x48-0x4D)
    KEY_UP, 0, KEY_LEFT, 0, KEY_RIGHT, 0, KEY_DOWN
};

// QWERTZ layout scancode to ASCII mapping (German)
static const uint16_t qwertz_layout[] = { 
    0, KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // Arrow keys (0x48-0x4D)
    KEY_UP, 0, KEY_LEFT, 0, KEY_RIGHT, 0, KEY_DOWN
};

// QWERTY shifted characters
static const uint16_t qwerty_shifted[] = { 
    0, KEY_ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // Arrow keys (unchanged with shift)
    KEY_UP, 0, KEY_LEFT, 0, KEY_RIGHT, 0, KEY_DOWN
};

// QWERTZ shifted characters  
static const uint16_t qwertz_shifted[] = { 
    0, KEY_ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Y', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // Arrow keys (unchanged with shift)
    KEY_UP, 0, KEY_LEFT, 0, KEY_RIGHT, 0, KEY_DOWN
};

// Shift key state
static int shift_pressed = 0;

// Function to set keyboard layout
void set_keyboard_layout(keyboard_layout_t layout) {
    current_layout = layout;
}

// Function to get current layout name
const char* get_layout_name() {
    return (current_layout == LAYOUT_QWERTY) ? "QWERTY" : "QWERTZ";
}

// Function to convert scancode to ASCII or special key code
uint16_t scancode_to_ascii(uint8_t scancode) {
    // Handle shift key presses/releases
    if (scancode == 0x2A || scancode == 0x36) { // Left/Right shift pressed
        shift_pressed = 1;
        return 0;
    }
    if (scancode == 0xAA || scancode == 0xB6) { // Left/Right shift released
        shift_pressed = 0;
        return 0;
    }
    
    // Extended key sequences (arrow keys, etc.)
    static int is_extended = 0;
    if (scancode == 0xE0) {
        is_extended = 1;
        return 0;
    }
    
    // Handle extended key releases
    if (is_extended && (scancode & 0x80)) {
        is_extended = 0;
        return 0;
    }
    
    // Handle extended keys
    if (is_extended) {
        is_extended = 0;
        switch (scancode) {
            case 0x48: return KEY_UP;
            case 0x50: return KEY_DOWN;
            case 0x4B: return KEY_LEFT;
            case 0x4D: return KEY_RIGHT;
            case 0x47: return KEY_HOME;
            case 0x4F: return KEY_END;
            case 0x49: return KEY_PGUP;
            case 0x51: return KEY_PGDN;
            case 0x53: return KEY_DEL;
            default: return 0;
        }
    }
    
    // Only process normal key presses (not releases)
    if (scancode & 0x80) {
        return 0;
    }
    
    // Select appropriate scancode table
    const uint16_t* layout_table;
    if (shift_pressed) {
        layout_table = (current_layout == LAYOUT_QWERTY) ? qwerty_shifted : qwertz_shifted;
    } else {
        layout_table = (current_layout == LAYOUT_QWERTY) ? qwerty_layout : qwertz_layout;
    }
    
    // Return ASCII character or special key code if scancode is valid
    if (scancode < 58) { // Max scancode we handle
        return layout_table[scancode];
    }
    
    return 0;
}

#endif // KEYBOARD_H