#include <stdint.h>
#include <stddef.h>
#include "terminal.h"
#include "keyboard.h"
#include "shutdown.h"
#include "reboot.h"
#include "timer.h"
#include "memory.h"
#include "boot.h"
#include "debug.h"
#include "command.h"
#include "opfs.h"
#include "minilibc.h"
#include "serial.h"
#include "net.h"

void draw_prompt(void) {
    terminal_set_color(0x0A);
    terminal_writestring("OPERATOR > ");
    terminal_set_color(0x07);
}

void draw_banner(void) {
    terminal_set_color(0x0B);
    terminal_writestring("                    operator version 2                    \n");
    terminal_writestring("         we have undergone a critical rewrite!             \n");
    terminal_set_color(0x07);
}

void print_status(const char* msg, int ok) {
    char buf[128];
    snprintf(buf, sizeof(buf), "[    0.000000] %s", msg);
    terminal_set_color(0x07);
    terminal_writestring(buf);
    int pad = 60 - strlen(buf);
    for (int i = 0; i < pad; i++) terminal_putchar(' ');
    if (ok) {
        terminal_set_color(0x0A); // Green
        terminal_writestring("[ OK ]\n");
    } else {
        terminal_set_color(0x0C); // Red
        terminal_writestring("[FAIL]\n");
    }
    terminal_set_color(0x07);
}

void kernel_module_selfcheck(void) {
    int errors = 0;
    print_status("Checking terminal module...", terminal_initialize && terminal_putchar && terminal_writestring && terminal_set_color);
    print_status("Checking keyboard module...", get_keyboard_char && set_keyboard_layout && get_layout_name && scancode_to_ascii);
    print_status("Checking shutdown module...", shutdown && halt_cpu);
    print_status("Checking reboot module...", reboot != NULL);
    print_status("Checking timer module...", timer_tick && get_system_ticks && delay_ms);
    print_status("Checking memory module...", malloc && test_memory && get_heap_used && get_heap_total);
    print_status("Checking boot module...", boot_sequence && halt_with_error && halt);
    print_status("Checking debug module...", draw_debug_info && toggle_debug_mode);
    print_status("Checking command module...", process_command != NULL);
    print_status("Checking OPFS module...", opfs_init && opfs_ls && opfs_cat && opfs_touch && opfs_edit && opfs_rm && opfs_mkdir && opfs_cd && opfs_pwd);
    print_status("Checking minilibc module...", memset && strlen && strcpy && strncmp && snprintf);
    print_status("Checking network module...", net_init && net_send && net_receive);
}

void kernel_main(void) {
    serial_init();
    terminal_initialize();
    terminal_set_color(0x0B);
    terminal_writestring("\n                         __           ____  ____\n");
    terminal_writestring(" ___  ___  ___ _______ _/ /____  ____/ __ \\/ __/\n");
    terminal_writestring("/ _ \\/ _ \\/ -_) __/ _ `/ __/ _ \\/ __/ /_/ /\\ \\  \n");
    terminal_writestring("\\___/ .__/\\__/_/  \\_,_/\\__/\\___/_/__\\____/___/  \n");
    terminal_writestring("   /_/                          /___/       \n\n");
    terminal_set_color(0x07);
    print_status("Operator OS kernel booting...", 1);
    print_status("Initializing modules...", 1);
    kernel_module_selfcheck();
    print_status("Initializing network...", 1);
    net_init();
    print_status("Running boot sequence...", 1);
    boot_sequence();
    print_status("Loading OPFS (Operator Pseudo Filesystem)...", 1);
    print_status("(This will soon be real.)", 1);
    opfs_load();
    print_status("System ready. Type 'help' for commands.", 1);
    draw_prompt();
    while (1) {
        uint16_t c = get_keyboard_char();
        if (c) {
            if (c == '\r' || c == '\n') {
                terminal_putchar('\n');
                input_buffer[input_pos] = 0;
                process_command();
                input_pos = 0;
                draw_prompt();
            } else if (c == '\b') {
                if (input_pos > 0) {
                    input_pos--;
                    terminal_putchar('\b');
                }
            } else if (c < 128 && input_pos < 255) {
                terminal_putchar((char)c);
                input_buffer[input_pos++] = (char)c;
            }
        }
    }
}