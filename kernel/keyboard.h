#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include "error.h"

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

// Keyboard-Initialisierung
int keyboard_init(void);

// Keyboard-Handler
void keyboard_handler(void);

// Keyboard-Eingabe
uint16_t keyboard_getchar(void);

// Keyboard-Funktionen
int keyboard_is_key_pressed(uint8_t scancode);
void keyboard_wait_for_key(void);

// Keyboard-Fehlerbehandlung
enum error_code_t keyboard_get_last_error(void);
const char* keyboard_get_error_message(void);

#endif // KEYBOARD_H