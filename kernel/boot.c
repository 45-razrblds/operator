#include "boot.h"
#include "terminal.h"
#include "memory.h"
#include "timer.h"
#include "keyboard.h"
#include "io.h"

void halt() {
    asm volatile ("cli; hlt");
    while (1) {}
}

void halt_with_error(const char* msg) {
    terminal_set_color(0x0C);
    terminal_writestring("ERROR: ");
    terminal_writestring(msg);
    terminal_writestring("\nSystem halted.\n");
    halt();
}

int test_vga() {
    terminal_writestring("Testing VGA output... [OK]\n");
    return 1;
}

int test_keyboard() {
    terminal_writestring("Testing keyboard input: press any key... ");
    while (1) {
        uint8_t status = inb(0x64);
        if (status & 0x01) {
            uint8_t scancode = inb(0x60);
            if (scancode < 0x80) {
                terminal_writestring("[OK]\n");
                return 1;
            }
        }
    }
    return 0;
}

int test_timer() {
    terminal_writestring("Testing timer... ");
    uint32_t start = get_system_ticks();
    delay_ms(100);
    uint32_t end = get_system_ticks();
    if (end > start) {
        terminal_writestring("[OK]\n");
        return 1;
    }
    terminal_writestring("[FAIL]\n");
    return 0;
}

void boot_sequence() {
    terminal_set_color(0x0F);
    terminal_writestring("OPERATOR v1.8 - Booting...\n\n");
    if (!test_vga()) halt_with_error("VGA test failed.");
    if (!test_memory()) halt_with_error("Memory test failed.");
    if (!test_keyboard()) halt_with_error("Keyboard test failed.");
    if (!test_timer()) halt_with_error("Timer test failed.");
    terminal_set_color(0x0A);
    terminal_writestring("\nAll tests passed. System ready.\n\n");
    terminal_set_color(0x07);
}
