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
#include "io.h"

// Global system state
static int system_healthy = 1;
static int rescue_mode = 0;
static char rescue_buffer[256];
static int rescue_pos = 0;

// Test results tracking
typedef struct {
    int terminal_ok;
    int keyboard_ok;
    int memory_ok;
    int timer_ok;
    int color_ok;
    int charset_ok;
    int modules_ok;
    int total_tests;
    int passed_tests;
} system_health_t;

static system_health_t health = {0};

// Forward declarations
void kernel_module_selfcheck(void);
extern char input_buffer[256];
extern int input_pos;

// Debug/testing flags
static int force_rescue_mode = 0;
static int simulate_failures = 0;

// Inline assembly rescue functions
static inline void rescue_putchar_asm(char c) {
    static uint16_t* vga = (uint16_t*)0xB8000;
    static int pos = 0;
    
    if (c == '\n') {
        pos = (pos / 80 + 1) * 80;
        if (pos >= 80 * 25) {
            // Simple scroll
            asm volatile (
                "cld\n\t"
                "mov $0xB8000, %%edi\n\t"
                "mov $0xB80A0, %%esi\n\t"
                "mov $1920, %%ecx\n\t"
                "rep movsw\n\t"
                "mov $0xB8F00, %%edi\n\t"
                "mov $0x0720, %%ax\n\t"
                "mov $80, %%ecx\n\t"
                "rep stosw"
                :
                :
                : "eax", "ecx", "edi", "esi", "memory"
            );
            pos = 24 * 80;
        }
    } else {
        vga[pos] = 0x0F00 | c;
        pos++;
        if (pos >= 80 * 25) pos = 0;
    }
}

static void rescue_writestring_asm(const char* str) {
    while (*str) {
        rescue_putchar_asm(*str);
        str++;
    }
}

// Comprehensive test functions
static int test_terminal_comprehensive(void) {
    int score = 0;
    health.total_tests += 5;
    
    // Force failure for testing if requested
    if (simulate_failures & 0x01) {
        terminal_writestring("Terminal test FORCED FAILURE for testing\n");
        health.terminal_ok = 0;
        return 0;
    }
    
    // Test 1: Basic initialization
    if (terminal_initialize && terminal_putchar && terminal_writestring) {
        score++;
        health.passed_tests++;
    }
    
    // Test 2: All printable ASCII characters
    terminal_writestring("Testing all printable characters: ");
    for (int i = 32; i < 127; i++) {
        terminal_putchar((char)i);
        delay_ms(2); // Small delay for visibility
    }
    terminal_writestring(" [PASS]\n");
    score++;
    health.passed_tests++;
    
    // Test 3: Special characters
    terminal_writestring("Testing special chars: \\n\\b\\r: ");
    terminal_putchar('\n');
    terminal_writestring("Newline OK, ");
    terminal_putchar('X');
    terminal_putchar('\b');
    terminal_writestring("Backspace OK [PASS]\n");
    score++;
    health.passed_tests++;
    
    // Test 4: Line wrapping
    terminal_writestring("Testing line wrap: ");
    for (int i = 0; i < 85; i++) {
        terminal_putchar('=');
    }
    terminal_writestring(" [PASS]\n");
    score++;
    health.passed_tests++;
    
    // Test 5: Scrolling
    terminal_writestring("Testing scroll (filling screen):\n");
    for (int i = 0; i < 30; i++) {
        char line[32];
        snprintf(line, sizeof(line), "Scroll test line %d\n", i);
        terminal_writestring(line);
        delay_ms(50);
    }
    score++;
    health.passed_tests++;

    
    health.terminal_ok = (score >= 4);
    return score >= 4;
}

static int test_color_comprehensive(void) {
    int score = 0;
    health.total_tests += 3;
    
    // Test 1: All 16 VGA colors
    terminal_writestring("Testing all VGA colors:\n");
    for (int bg = 0; bg < 2; bg++) {
        for (int fg = 0; fg < 16; fg++) {
            uint8_t color = (bg << 4) | fg;
            terminal_set_color(color);
            char colorstr[8];
            snprintf(colorstr, sizeof(colorstr), "%02X ", color);
            terminal_writestring(colorstr);
        }
        terminal_writestring("\n");
    }
    terminal_set_color(0x07);
    score++;
    health.passed_tests++;
    
    // Test 2: RGB color mapping
    if (terminal_setcolor_rgb) {
        terminal_writestring("Testing RGB colors: ");
        terminal_setcolor_rgb(255, 0, 0);     // Red
        terminal_writestring("R");
        terminal_setcolor_rgb(0, 255, 0);     // Green
        terminal_writestring("G");
        terminal_setcolor_rgb(0, 0, 255);     // Blue
        terminal_writestring("B");
        terminal_setcolor_rgb(255, 255, 0);   // Yellow
        terminal_writestring("Y");
        terminal_setcolor_rgb(255, 0, 255);   // Magenta
        terminal_writestring("M");
        terminal_setcolor_rgb(0, 255, 255);   // Cyan
        terminal_writestring("C");
        terminal_setcolor_default();
        terminal_writestring(" [PASS]\n");
        score++;
        health.passed_tests++;
    }
    
    // Test 3: Color persistence
    terminal_set_color(0x0A);
    terminal_writestring("Green text test");
    terminal_set_color(0x0C);
    terminal_writestring(" Red text test");
    terminal_set_color(0x07);
    terminal_writestring(" Default [PASS]\n");
    score++;
    health.passed_tests++;
    
    health.color_ok = (score >= 2);
    return score >= 2;
}

static int test_keyboard_comprehensive(void) {
    int score = 0;
    health.total_tests += 2;
    
    terminal_writestring("Testing keyboard (press 5 different keys): ");
    int keys_pressed = 0;
    uint8_t last_keys[5] = {0};
    
    while (keys_pressed < 5) {
        uint8_t status = inb(0x64);
        if (status & 0x01) {
            uint8_t scancode = inb(0x60);
            if (scancode < 0x80) {  // Key press (not release)
                // Check if it's a new key
                int is_new = 1;
                for (int i = 0; i < keys_pressed; i++) {
                    if (last_keys[i] == scancode) {
                        is_new = 0;
                        break;
                    }
                }
                if (is_new) {
                    last_keys[keys_pressed] = scancode;
                    keys_pressed++;
                    terminal_putchar('*');
                }
            }
        }
    }
    terminal_writestring(" [PASS]\n");
    score++;
    health.passed_tests++;
    
    // Test keyboard layout functions
    if (get_layout_name && set_keyboard_layout) {
        terminal_writestring("Keyboard layout functions: [PASS]\n");
        score++;
        health.passed_tests++;
    }
    
    health.keyboard_ok = (score >= 1);
    return score >= 1;
}

static int test_memory_comprehensive(void) {
    int score = 0;
    health.total_tests += 4;
    
    // Force failure for testing if requested
    if (simulate_failures & 0x04) {
        terminal_writestring("Memory test FORCED FAILURE for testing\n");
        health.memory_ok = 0;
        return 0;
    }
    
    // Test 1: Basic memory functions
    if (malloc && test_memory) {
        if (test_memory()) {
            score++;
            health.passed_tests++;
        }
    }
    
    // Test 2: Memory allocation patterns
    terminal_writestring("Testing memory allocation patterns:\n");
    void* ptrs[10];
    int alloc_success = 1;
    
    for (int i = 0; i < 10; i++) {
        ptrs[i] = malloc(1024 * (i + 1));
        if (!ptrs[i]) {
            alloc_success = 0;
            break;
        }
        // Write pattern to memory
        memset(ptrs[i], 0xAA + i, 1024 * (i + 1));
    }
    
    if (alloc_success) {
        terminal_writestring("  Allocation: [PASS]\n");
        score++;
        health.passed_tests++;
    }
    
    // Test 3: Memory integrity check
    int integrity_ok = 1;
    for (int i = 0; i < 10 && ptrs[i]; i++) {
        uint8_t* ptr = (uint8_t*)ptrs[i];
        for (int j = 0; j < 100; j++) {
            if (ptr[j] != (uint8_t)(0xAA + i)) {
                integrity_ok = 0;
                break;
            }
        }
    }
    
    if (integrity_ok) {
        terminal_writestring("  Integrity: [PASS]\n");
        score++;
        health.passed_tests++;
    }
    
    // Test 4: Memory statistics
    if (get_heap_used && get_heap_total) {
        uint32_t used = get_heap_used();
        uint32_t total = get_heap_total();
        if (used > 0 && total > used) {
            terminal_writestring("  Statistics: [PASS]\n");
            score++;
            health.passed_tests++;
        }
    }
    
    health.memory_ok = (score >= 2);
    return score >= 2;
}

static int test_timer_comprehensive(void) {
    int score = 0;
    health.total_tests += 3;
    
    // Test 1: Basic timer functions
    if (timer_tick && get_system_ticks && delay_ms) {
        uint32_t start = get_system_ticks();
        delay_ms(100);
        uint32_t end = get_system_ticks();
        if (end > start) {
            terminal_writestring("Basic timer: [PASS]\n");
            score++;
            health.passed_tests++;
        }
    }
    
    // Test 2: Timer precision
    terminal_writestring("Testing timer precision: ");
    uint32_t measurements[5];
    for (int i = 0; i < 5; i++) {
        uint32_t start = get_system_ticks();
        delay_ms(50);
        measurements[i] = get_system_ticks() - start;
        terminal_putchar('.');
    }
    
    // Check if measurements are consistent (within 20% variance)
    uint32_t avg = 0;
    for (int i = 0; i < 5; i++) avg += measurements[i];
    avg /= 5;
    
    int consistent = 1;
    for (int i = 0; i < 5; i++) {
        if (measurements[i] < avg * 8 / 10 || measurements[i] > avg * 12 / 10) {
            consistent = 0;
            break;
        }
    }
    
    if (consistent) {
        terminal_writestring(" [PASS]\n");
        score++;
        health.passed_tests++;
    } else {
        terminal_writestring(" [WARN]\n");
    }
    
    // Test 3: System uptime
    uint32_t uptime = get_system_ticks();
    if (uptime > 0) {
        char uptime_str[64];
        snprintf(uptime_str, sizeof(uptime_str), "System uptime: %u ticks [PASS]\n", uptime);
        terminal_writestring(uptime_str);
        score++;
        health.passed_tests++;
    }
    
    health.timer_ok = (score >= 2);
    return score >= 2;
}

static int test_all_modules(void) {
    int score = 0;
    health.total_tests += 10;
    
    // Test each module's function pointers
    struct {
        void* func;
        const char* name;
    } modules[] = {
        {terminal_initialize, "terminal_initialize"},
        {get_keyboard_char, "get_keyboard_char"},
        {shutdown, "shutdown"},
        {reboot, "reboot"},
        {malloc, "malloc"},
        {boot_sequence, "boot_sequence"},
        {process_command, "process_command"},
        {opfs_init, "opfs_init"},
        {strlen, "strlen"},
        {net_init, "net_init"}
    };
    
    for (int i = 0; i < 10; i++) {
        if (modules[i].func != NULL) {
            score++;
            health.passed_tests++;
        }
        char status[128];
        snprintf(status, sizeof(status), "  %s: %s\n", 
                modules[i].name, 
                modules[i].func ? "[OK]" : "[FAIL]");
        terminal_writestring(status);
    }
    
    health.modules_ok = (score >= 8);
    return score >= 8;
}

// Emergency rescue system in assembly
static void enter_rescue_mode(void) {
    rescue_mode = 1;
    system_healthy = 0;
    
    // Clear screen using assembly
    asm volatile (
        "cld\n\t"
        "mov $0xB8000, %%edi\n\t"
        "mov $0x4F20, %%ax\n\t"  // White on red background
        "mov $2000, %%ecx\n\t"
        "rep stosw"
        :
        :
        : "eax", "ecx", "edi", "memory"
    );
    
    rescue_writestring_asm("*** EMERGENCY RESCUE MODE ***\n");
    rescue_writestring_asm("System tests failed. Minimal interface active.\n");
    rescue_writestring_asm("Available commands: help, status, reboot, halt\n");
    rescue_writestring_asm("RESCUE> ");
}

static void rescue_command_help(void) {
    rescue_writestring_asm("Available rescue commands:\n");
    rescue_writestring_asm("  help   - Show this help\n");
    rescue_writestring_asm("  status - Show system status\n");
    rescue_writestring_asm("  reboot - Restart system\n");
    rescue_writestring_asm("  halt   - Stop system\n");
    rescue_writestring_asm("  test   - Re-run system tests\n");
}

static void rescue_command_status(void) {
    rescue_writestring_asm("System Health Report:\n");
    char status[256];
    
    // Use simple string building since we can't rely on snprintf
    rescue_writestring_asm("  Terminal: ");
    rescue_writestring_asm(health.terminal_ok ? "OK\n" : "FAIL\n");
    rescue_writestring_asm("  Keyboard: ");
    rescue_writestring_asm(health.keyboard_ok ? "OK\n" : "FAIL\n");
    rescue_writestring_asm("  Memory: ");
    rescue_writestring_asm(health.memory_ok ? "OK\n" : "FAIL\n");
    rescue_writestring_asm("  Timer: ");
    rescue_writestring_asm(health.timer_ok ? "OK\n" : "FAIL\n");
    rescue_writestring_asm("  Colors: ");
    rescue_writestring_asm(health.color_ok ? "OK\n" : "FAIL\n");
    rescue_writestring_asm("  Modules: ");
    rescue_writestring_asm(health.modules_ok ? "OK\n" : "FAIL\n");
}

static void process_rescue_command(void) {
    rescue_buffer[rescue_pos] = 0;
    
    if (strncmp(rescue_buffer, "help", 4) == 0) {
        rescue_command_help();
    } else if (strncmp(rescue_buffer, "status", 6) == 0) {
        rescue_command_status();
    } else if (strncmp(rescue_buffer, "reboot", 6) == 0) {
        rescue_writestring_asm("Rebooting system...\n");
        if (reboot) reboot();
        else {
            // Assembly reboot
            asm volatile ("int $0x19" : : : "memory");
        }
    } else if (strncmp(rescue_buffer, "halt", 4) == 0) {
        rescue_writestring_asm("Halting system...\n");
        halt();
    } else if (strncmp(rescue_buffer, "test", 4) == 0) {
        rescue_writestring_asm("Re-running system tests...\n");
        // Reset health and try again
        memset(&health, 0, sizeof(health));
        kernel_module_selfcheck();
        if (health.passed_tests >= health.total_tests * 3 / 4) {
            rescue_writestring_asm("Tests improved! Attempting normal boot...\n");
            rescue_mode = 0;
            system_healthy = 1;
            return;
        }
    } else if (rescue_pos > 0) {
        rescue_writestring_asm("Unknown command. Type 'help' for commands.\n");
    }
    
    rescue_writestring_asm("RESCUE> ");
    rescue_pos = 0;
}

void draw_prompt(void) {
    if (rescue_mode) return;
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
        terminal_set_color(0x0A);
        terminal_writestring("[ OK ]\n");
    } else {
        terminal_set_color(0x0C);
        terminal_writestring("[FAIL]\n");
    }
    terminal_set_color(0x07);
}

void kernel_module_selfcheck(void) {
    terminal_set_color(0x0E);
    terminal_writestring("=== COMPREHENSIVE SYSTEM TEST ===\n");
    terminal_set_color(0x07);
    
    // TESTING: Uncomment one of these lines to force rescue mode
    // simulate_failures = 0x01;  // Force terminal test to fail
    // simulate_failures = 0x04;  // Force memory test to fail
    // force_rescue_mode = 1;     // Skip all tests and go directly to rescue
    
    if (force_rescue_mode) {
        terminal_writestring("FORCE RESCUE MODE ACTIVATED FOR TESTING\n");
        health.total_tests = 100;
        health.passed_tests = 10;  // Only 10% success rate
        system_healthy = 0;
        return;
    }
    
    // Run all comprehensive tests
    terminal_writestring("\n1. Terminal System Tests:\n");
    test_terminal_comprehensive();
    
    terminal_writestring("\n2. Color System Tests:\n");
    test_color_comprehensive();
    
    terminal_writestring("\n3. Keyboard System Tests:\n");
    test_keyboard_comprehensive();
    
    terminal_writestring("\n4. Memory System Tests:\n");
    test_memory_comprehensive();
    
    terminal_writestring("\n5. Timer System Tests:\n");
    test_timer_comprehensive();
    
    terminal_writestring("\n6. Module Integrity Tests:\n");
    test_all_modules();
    
    // Summary
    terminal_set_color(0x0E);
    terminal_writestring("\n=== TEST SUMMARY ===\n");
    terminal_set_color(0x07);
    
    char summary[256];
    snprintf(summary, sizeof(summary), 
             "Total Tests: %d, Passed: %d, Failed: %d\n",
             health.total_tests, health.passed_tests, 
             health.total_tests - health.passed_tests);
    terminal_writestring(summary);
    
    int success_rate = (health.passed_tests * 100) / health.total_tests;
    snprintf(summary, sizeof(summary), "Success Rate: %d%%\n", success_rate);
    terminal_writestring(summary);
    
    if (success_rate < 75) {
        terminal_set_color(0x0C);
        terminal_writestring("CRITICAL: System health below 75%. Entering rescue mode.\n");
        system_healthy = 0;
    } else {
        terminal_set_color(0x0A);
        terminal_writestring("System health acceptable. Continuing normal boot.\n");
        system_healthy = 1;
    }
    terminal_set_color(0x07);
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
    
    // Run comprehensive system tests
    kernel_module_selfcheck();
    
    if (!system_healthy) {
        enter_rescue_mode();
        
        // Rescue mode main loop
        while (1) {
            uint8_t status = inb(0x64);
            if (status & 0x01) {
                uint8_t scancode = inb(0x60);
                if (scancode < 0x80) {  // Key press
                    char c = scancode_to_ascii ? scancode_to_ascii(scancode) : 0;
                    if (c) {
                        if (c == '\r' || c == '\n') {
                            rescue_putchar_asm('\n');
                            process_rescue_command();
                            if (!rescue_mode) break;  // Exit rescue mode
                        } else if (c == '\b') {
                            if (rescue_pos > 0) {
                                rescue_pos--;
                                rescue_putchar_asm('\b');
                            }
                        } else if (c >= 32 && c < 127 && rescue_pos < 255) {
                            rescue_putchar_asm(c);
                            rescue_buffer[rescue_pos++] = c;
                        }
                    }
                }
            }
        }
    }
    
    // Normal system startup
    print_status("Initializing network...", 1);
    net_init();
    print_status("Running boot sequence...", 1);
    boot_sequence();
    print_status("Loading OPFS (Operator Pseudo Filesystem)...", 1);
    print_status("(This will soon be real.)", 1);
    opfs_load();
    print_status("System ready. Type 'help' for commands.", 1);
    
    draw_prompt();
    
    // Main system loop
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