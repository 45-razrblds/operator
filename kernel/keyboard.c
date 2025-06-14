#include "keyboard.h"
#include "error.h"
#include "minilibc.h"
#include "io.h"
#include "timer.h"

#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64
#define KEYBOARD_COMMAND_PORT 0x64

#define KEYBOARD_BUFFER_SIZE 256
static uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static int keyboard_buffer_head = 0;
static int keyboard_buffer_tail = 0;

static enum error_code_t last_error = ERR_NONE;

int keyboard_init(void) {
    // Warte auf Keyboard-Controller
    int timeout = 1000;
    while (timeout--) {
        if ((inb(KEYBOARD_STATUS_PORT) & 0x02) == 0) {
            break;
        }
        // Warte 1ms
        for(volatile int i = 0; i < 1000000; i++);
    }
    
    if (timeout == 0) {
        ERROR_SET(ERR_DEVICE_BUSY, "Keyboard controller not responding");
        return 0;
    }
    
    // Setze Keyboard-Controller
    outb(KEYBOARD_COMMAND_PORT, 0xAE); // Enable keyboard
    outb(KEYBOARD_COMMAND_PORT, 0x20); // Get command byte
    uint8_t status = inb(KEYBOARD_DATA_PORT);
    status |= 0x01; // Enable keyboard interrupt
    outb(KEYBOARD_COMMAND_PORT, 0x60); // Set command byte
    outb(KEYBOARD_DATA_PORT, status);
    
    return 1;
}

void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    uint16_t ascii = scancode_to_ascii(scancode);
    
    if (ascii) {
        int next = (keyboard_buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
        if (next != keyboard_buffer_tail) {
            keyboard_buffer[keyboard_buffer_head] = ascii;
            keyboard_buffer_head = next;
        } else {
            ERROR_SET(ERR_DEVICE_BUSY, "Keyboard buffer overflow");
        }
    }
}

uint16_t keyboard_getchar(void) {
    if (keyboard_buffer_head == keyboard_buffer_tail) {
        return 0;
    }
    
    uint16_t result = keyboard_buffer[keyboard_buffer_tail];
    keyboard_buffer_tail = (keyboard_buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
    return result;
}

int keyboard_is_key_pressed(uint8_t scancode) {
    if (!(inb(KEYBOARD_STATUS_PORT) & 0x01)) {
        return 0;
    }
    
    uint8_t current_scancode = inb(KEYBOARD_DATA_PORT);
    return (current_scancode == scancode);
}

void keyboard_wait_for_key(void) {
    while (!(inb(KEYBOARD_STATUS_PORT) & 0x01)) {
        asm volatile("hlt");
    }
}

enum error_code_t keyboard_get_last_error(void) {
    return last_error;
}

const char* keyboard_get_error_message(void) {
    return error_to_string(last_error);
}

// Scancode-Tabelle für US-Layout
static const uint16_t scancode_table[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0
};

uint16_t scancode_to_ascii(uint8_t scancode) {
    if (scancode >= sizeof(scancode_table) / sizeof(scancode_table[0])) {
        return 0;
    }
    return scancode_table[scancode];
}
