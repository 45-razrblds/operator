#include "keyboard.h"
#include "io.h"

static keyboard_layout_t current_layout = LAYOUT_QWERTY;
static int shift_pressed = 0;

static const uint16_t qwerty_layout[] = {
    0, KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    KEY_UP, 0, KEY_LEFT, 0, KEY_RIGHT, 0, KEY_DOWN
};
static const uint16_t qwertz_layout[] = {
    0, KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    KEY_UP, 0, KEY_LEFT, 0, KEY_RIGHT, 0, KEY_DOWN
};
static const uint16_t qwerty_shifted[] = {
    0, KEY_ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    KEY_UP, 0, KEY_LEFT, 0, KEY_RIGHT, 0, KEY_DOWN
};
static const uint16_t qwertz_shifted[] = {
    0, KEY_ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Y', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    KEY_UP, 0, KEY_LEFT, 0, KEY_RIGHT, 0, KEY_DOWN
};

void set_keyboard_layout(keyboard_layout_t layout) {
    current_layout = layout;
}

const char* get_layout_name(void) {
    return (current_layout == LAYOUT_QWERTY) ? "QWERTY" : "QWERTZ";
}

uint16_t scancode_to_ascii(uint8_t scancode) {
    // Handle special case for ESC key (scancode 0x01)
    if (scancode == 0x01) return KEY_ESC;

    if (scancode == 0x2A || scancode == 0x36) { shift_pressed = 1; return 0; }
    if (scancode == 0xAA || scancode == 0xB6) { shift_pressed = 0; return 0; }
    static int is_extended = 0;
    if (scancode == 0xE0) { is_extended = 1; return 0; }
    if (is_extended && (scancode & 0x80)) { is_extended = 0; return 0; }
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
    if (scancode & 0x80) return 0;
    const uint16_t* layout_table;
    if (shift_pressed) {
        layout_table = (current_layout == LAYOUT_QWERTY) ? qwerty_shifted : qwertz_shifted;
    } else {
        layout_table = (current_layout == LAYOUT_QWERTY) ? qwerty_layout : qwertz_layout;
    }
    if (scancode < 58) return layout_table[scancode];
    return 0;
}

uint16_t get_keyboard_char(void) {
    // Poll the keyboard controller for a keypress
    if ((inb(0x64) & 0x01) == 0)
        return 0; // No key available
    uint8_t scancode = inb(0x60);
    return scancode_to_ascii(scancode);
}
