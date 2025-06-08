#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

// Keyboard layout types
typedef enum {
    LAYOUT_QWERTY = 0,
    LAYOUT_QWERTZ = 1
} keyboard_layout_t;

// Current keyboard layout
static keyboard_layout_t current_layout = LAYOUT_QWERTY;

// QWERTY layout scancode to ASCII mapping
static const char qwerty_layout[] = { 
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

// QWERTZ layout scancode to ASCII mapping (German)
static const char qwertz_layout[] = { 
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

// QWERTY shifted characters
static const char qwerty_shifted[] = { 
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

// QWERTZ shifted characters  
static const char qwertz_shifted[] = { 
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Y', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
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

// Function to convert scancode to ASCII based on current layout
char scancode_to_ascii(uint8_t scancode) {
    // Handle shift key presses/releases
    if (scancode == 0x2A || scancode == 0x36) { // Left/Right shift pressed
        shift_pressed = 1;
        return 0;
    }
    if (scancode == 0xAA || scancode == 0xB6) { // Left/Right shift released
        shift_pressed = 0;
        return 0;
    }
    
    // Only process key presses (not releases)
    if (scancode & 0x80) {
        return 0;
    }
    
    // Select appropriate scancode table
    const char* layout_table;
    if (shift_pressed) {
        layout_table = (current_layout == LAYOUT_QWERTY) ? qwerty_shifted : qwertz_shifted;
    } else {
        layout_table = (current_layout == LAYOUT_QWERTY) ? qwerty_layout : qwertz_layout;
    }
    
    // Return ASCII character if scancode is valid
    if (scancode < 58) { // Max scancode we handle
        return layout_table[scancode];
    }
    
    return 0;
}

#endif // KEYBOARD_H