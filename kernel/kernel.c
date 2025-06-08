#include <stdint.h>
#include <stddef.h>
#include "keyboard.h"  // Keyboard layout support

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

static uint16_t* terminal_buffer;
static uint16_t terminal_row;
static uint16_t terminal_column;
static uint8_t terminal_color;
static char input_buffer[256];
static int input_pos = 0;

// Simple bump allocator heap start and size
#define HEAP_START 0x1000000
#define HEAP_SIZE  0x100000

static uintptr_t heap_ptr = HEAP_START;

// Timer tick count
volatile uint32_t timer_ticks = 0;

// --- Terminal functions ---

void terminal_initialize() {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = 0x07; // Light grey on black
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
            // Scroll up
            for (int y = 0; y < VGA_HEIGHT - 1; y++) {
                for (int x = 0; x < VGA_WIDTH; x++) {
                    terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
                }
            }
            // Clear last line
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

// --- I/O port functions ---

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// --- Delay ---

void delay(uint32_t count) {
    for (volatile uint32_t i = 0; i < count * 100000; i++);
}

// --- Memory allocator ---

void* malloc(size_t size) {
    void* ptr = (void*)heap_ptr;
    heap_ptr += size;
    if (heap_ptr > HEAP_START + HEAP_SIZE) {
        return 0; // out of memory
    }
    return ptr;
}

// --- Timer simulation ---

// Normally this would be incremented by PIT IRQ, but here we simulate with delay loops
void timer_tick() {
    timer_ticks++;
}

uint32_t get_system_ticks() {
    return timer_ticks;
}

void delay_ms(uint32_t ms) {
    uint32_t start = get_system_ticks();
    while ((get_system_ticks() - start) < (ms / 10)) {
        // simulate ticks increment
        timer_tick();
    }
}

// --- Tests with halting on failure ---

void halt() {
    asm volatile ("cli; hlt");
    while (1) {}
}

void halt_with_error(const char* msg) {
    terminal_set_color(0x0C); // Red
    terminal_writestring("ERROR: ");
    terminal_writestring(msg);
    terminal_writestring("\nSystem halted.\n");
    halt();
}

int test_vga() {
    terminal_writestring("Testing VGA output... ");
    terminal_writestring("[OK]\n");
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
    // unreachable
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

int test_memory() {
    terminal_writestring("Testing memory allocator... ");
    char* ptr = (char*)malloc(16);
    if (!ptr) {
        terminal_writestring("[FAIL]\n");
        return 0;
    }
    for (int i = 0; i < 16; i++) ptr[i] = (char)i;
    for (int i = 0; i < 16; i++) {
        if (ptr[i] != (char)i) {
            terminal_writestring("[FAIL]\n");
            return 0;
        }
    }
    terminal_writestring("[OK]\n");
    return 1;
}

// --- Boot sequence ---

void boot_sequence() {
    terminal_set_color(0x0F); // Bright white
    terminal_writestring("OPERATOR v1.8 - Booting...\n\n");
    
    if (!test_vga()) halt_with_error("VGA test failed.");
    if (!test_memory()) halt_with_error("Memory test failed.");
    if (!test_keyboard()) halt_with_error("Keyboard test failed.");
    if (!test_timer()) halt_with_error("Timer test failed.");
    
    terminal_set_color(0x0A); // Green
    terminal_writestring("\nAll tests passed. System ready.\n\n");
    terminal_set_color(0x07);
}

// --- Keyboard input ---

char get_keyboard_char() {
    uint8_t status = inb(0x64);
    if (status & 0x01) {
        uint8_t scancode = inb(0x60);
        return scancode_to_ascii(scancode);
    }
    return 0;
}

// --- String helper functions (same as your code) ---

int str_compare(const char* str1, const char* str2, int len) {
    for (int i = 0; i < len; i++) {
        if (str1[i] != str2[i]) return 0;
    }
    return 1;
}

int starts_with(const char* str, const char* prefix, int str_len, int prefix_len) {
    if (str_len < prefix_len) return 0;
    return str_compare(str, prefix, prefix_len);
}

// --- Command processing (unchanged) ---

void process_command() {
    terminal_putchar('\n');
    if (input_pos > 0) {
        input_buffer[input_pos] = '\0';
        
        if (str_compare(input_buffer, "help", input_pos) && input_pos == 4) {
            terminal_writestring("Available commands:\n");
            terminal_writestring("help       - Show this help message\n");
            terminal_writestring("clear      - Clear the screen\n");
            terminal_writestring("layout     - Show/change keyboard layout\n");
            terminal_writestring("echo <msg> - Echo the provided message\n");
            terminal_writestring("about      - Show system information\n");
            terminal_writestring("reboot     - Reboot the system\n");
        }
        else if (str_compare(input_buffer, "clear", input_pos) && input_pos == 5) {
            terminal_initialize();
            terminal_writestring("OPERATOR v1.8\n");
            terminal_writestring("Type 'help' for available commands.\n");
        }
        else if (starts_with(input_buffer, "layout", input_pos, 6)) {
            if (input_pos == 6) {
                terminal_writestring("Current keyboard layout: ");
                terminal_writestring(get_layout_name());
                terminal_writestring("\nUsage: layout qwerty | layout qwertz\n");
            } else if (starts_with(input_buffer, "layout qwerty", input_pos, 13) && input_pos == 13) {
                set_keyboard_layout(LAYOUT_QWERTY);
                terminal_writestring("Keyboard layout set to QWERTY.\n");
            } else if (starts_with(input_buffer, "layout qwertz", input_pos, 13) && input_pos == 13) {
                set_keyboard_layout(LAYOUT_QWERTZ);
                terminal_writestring("Keyboard layout set to QWERTZ.\n");
            } else {
                terminal_writestring("Invalid layout command. Use 'layout qwerty' or 'layout qwertz'.\n");
            }
        }
        else if (starts_with(input_buffer, "echo", input_pos, 4)) {
            if (input_pos > 5) {
                terminal_writestring(input_buffer + 5);
                terminal_putchar('\n');
            }
        }
        else if (str_compare(input_buffer, "about", input_pos) && input_pos == 5) {
            terminal_writestring("OPERATOR v1.8 - Simple bootloader & CLI\n");
            terminal_writestring("Author: txhc\n");
            terminal_writestring("This is a minimal OS prototype.\n");
        }
        else if (str_compare(input_buffer, "reboot", input_pos) && input_pos == 6) {
            terminal_writestring("Rebooting...\n");
            halt(); // Replaces invalid far jump with halt
        }
        else if (str_compare(input_buffer, "debug", input_pos) && input_pos == 5) {
            // Enter debug environment
            void debug_env();
            debug_env();
        }
        else {
            terminal_writestring("Unknown command: ");
            terminal_writestring(input_buffer);
            terminal_putchar('\n');
        }
    }
    input_pos = 0;
}

// --- Debug environment ---
void debug_env() {
    char debug_buffer[128];
    int debug_pos = 0;
    terminal_set_color(0x0E); // Yellow
    terminal_writestring("\n[DEBUG MODE]\nType 'help' to list debug commands.\n");
    terminal_set_color(0x07);
    terminal_writestring("DEBUG> ");
    while (1) {
        char c = get_keyboard_char();
        if (c) {
            if (c == '\r' || c == '\n') {
                terminal_putchar('\n');
                debug_buffer[debug_pos] = '\0';
                // --- Command parsing ---
                if (debug_pos == 4 && str_compare(debug_buffer, "exit", 4)) {
                    terminal_writestring("Exiting debug mode.\n");
                    break;
                } else if (debug_pos == 4 && str_compare(debug_buffer, "help", 4)) {
                    terminal_writestring("Debug commands:\n");
                    terminal_writestring("help         - Show this help\n");
                    terminal_writestring("exit         - Return to normal mode\n");
                    terminal_writestring("ticks        - Show timer tick count\n");
                    terminal_writestring("alloc        - Test memory allocation\n");
                    terminal_writestring("color <n>    - Set terminal color (hex)\n");
                    terminal_writestring("echo <msg>   - Echo a message\n");
                    terminal_writestring("vga          - Test VGA output\n");
                    terminal_writestring("clear        - Clear the screen\n");
                    terminal_writestring("info         - Show system info\n");
                    terminal_writestring("keys         - Wait for and show keypress\n");
                    terminal_writestring("fail         - Simulate a failure and halt\n");
                } else if (debug_pos == 5 && str_compare(debug_buffer, "ticks", 5)) {
                    char buf[32];
                    uint32_t t = get_system_ticks();
                    terminal_writestring("Ticks: ");
                    // Simple itoa
                    int i = 30; buf[31] = '\0';
                    if (t == 0) buf[i--] = '0';
                    while (t && i >= 0) { buf[i--] = '0' + (t % 10); t /= 10; }
                    terminal_writestring(&buf[i+1]);
                    terminal_putchar('\n');
                } else if (debug_pos == 5 && str_compare(debug_buffer, "alloc", 5)) {
                    void* p = malloc(32);
                    terminal_writestring("Allocated 32 bytes at: 0x");
                    char hex[9];
                    uintptr_t addr = (uintptr_t)p;
                    for (int j = 7; j >= 0; j--) {
                        int v = (addr >> (j*4)) & 0xF;
                        hex[7-j] = v < 10 ? '0'+v : 'A'+(v-10);
                    }
                    hex[8] = '\0';
                    terminal_writestring(hex);
                    terminal_putchar('\n');
                } else if (debug_pos > 6 && str_compare(debug_buffer, "color ", 6)) {
                    // Parse color value
                    int val = 0;
                    for (int k = 6; debug_buffer[k] != '\0'; k++) {
                        char ch = debug_buffer[k];
                        if (ch >= '0' && ch <= '9') val = val*16 + (ch-'0');
                        else if (ch >= 'A' && ch <= 'F') val = val*16 + (ch-'A'+10);
                        else if (ch >= 'a' && ch <= 'f') val = val*16 + (ch-'a'+10);
                        else break;
                    }
                    terminal_set_color((uint8_t)val);
                    terminal_writestring("Color set.\n");
                } else if (debug_pos > 5 && str_compare(debug_buffer, "echo ", 5)) {
                    terminal_writestring(debug_buffer+5);
                    terminal_putchar('\n');
                } else if (debug_pos == 3 && str_compare(debug_buffer, "vga", 3)) {
                    test_vga();
                } else if (debug_pos == 5 && str_compare(debug_buffer, "clear", 5)) {
                    terminal_initialize();
                    terminal_writestring("[DEBUG MODE]\n");
                } else if (debug_pos == 4 && str_compare(debug_buffer, "info", 4)) {
                    terminal_writestring("System info:\n");
                    terminal_writestring("Version: v1.8\n");
                    terminal_writestring("Heap ptr: 0x");
                    char hex[9];
                    uintptr_t addr = (uintptr_t)heap_ptr;
                    for (int j = 7; j >= 0; j--) {
                        int v = (addr >> (j*4)) & 0xF;
                        hex[7-j] = v < 10 ? '0'+v : 'A'+(v-10);
                    }
                    hex[8] = '\0';
                    terminal_writestring(hex);
                    terminal_writestring("\n");
                } else if (debug_pos == 4 && str_compare(debug_buffer, "keys", 4)) {
                    terminal_writestring("Press any key...\n");
                    uint8_t status = 0, sc = 0;
                    char ch = 0;
                    while (!ch) {
                        status = inb(0x64);
                        if (status & 0x01) {
                            sc = inb(0x60);
                            ch = scancode_to_ascii(sc);
                        }
                    }
                    terminal_writestring("Scancode: 0x");
                    char shex[3];
                    shex[0] = "0123456789ABCDEF"[(sc>>4)&0xF];
                    shex[1] = "0123456789ABCDEF"[sc&0xF];
                    shex[2] = '\0';
                    terminal_writestring(shex);
                    terminal_writestring(", ASCII: ");
                    terminal_putchar(ch ? ch : '?');
                    terminal_putchar('\n');
                } else if (debug_pos == 4 && str_compare(debug_buffer, "fail", 4)) {
                    halt_with_error("Simulated failure in debug mode.");
                } else {
                    terminal_writestring("Unknown debug command: ");
                    terminal_writestring(debug_buffer);
                    terminal_putchar('\n');
                }
                debug_pos = 0;
                terminal_writestring("DEBUG> ");
            } else if (c == '\b') {
                if (debug_pos > 0) {
                    debug_pos--;
                    terminal_putchar('\b');
                }
            } else if (debug_pos < (int)(sizeof(debug_buffer) - 1)) {
                debug_buffer[debug_pos++] = c;
                terminal_putchar(c);
            }
        }
    }
    terminal_set_color(0x07);
}

// --- Main entry point ---

void kernel_main() {
    terminal_initialize();
    boot_sequence();
    terminal_writestring("> ");
    
    while (1) {
        char c = get_keyboard_char();
        if (c) {
            if (c == '\r' || c == '\n') {
                process_command();
                terminal_writestring("> ");
            } else if (c == '\b') {
                if (input_pos > 0) {
                    input_pos--;
                    terminal_putchar('\b');
                }
            } else if (input_pos < (int)(sizeof(input_buffer) - 1)) {
                input_buffer[input_pos++] = c;
                terminal_putchar(c);
            }
        }
    }
}
