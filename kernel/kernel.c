#include <stdint.h>
#include "keyboard.h"  // Include our keyboard layout support

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

static uint16_t* terminal_buffer;
static uint16_t terminal_row;
static uint16_t terminal_column;
static uint8_t terminal_color;
static char input_buffer[256];
static int input_pos = 0;
static uint32_t boot_time = 0;

void terminal_initialize() {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = 0x07; // Light grey on black
    terminal_buffer = (uint16_t*) VGA_MEMORY;
    
    // Clear the screen
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
                terminal_buffer[(VGA_HEIGHT-1) * VGA_WIDTH + x] = (terminal_color << 8) | ' ';
            }
            terminal_row--;
        }
        return;
    }
    
    if (c == '\b') { // Backspace
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

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Simple delay function
void delay(uint32_t count) {
    for (volatile uint32_t i = 0; i < count * 100000; i++);
}

// Boot sequence animation
void boot_sequence() {
    terminal_set_color(0x0F); // Bright white
    terminal_writestring("SimpleOS v1.0 - Starting Boot Sequence...\n\n");
    delay(20);
    
    terminal_set_color(0x0A); // Green
    terminal_writestring("[OK] Initializing VGA Display\n");
    delay(15);
    
    terminal_writestring("[OK] Setting up Memory Management\n");
    delay(15);
    
    terminal_writestring("[OK] Loading Keyboard Driver\n");
    delay(15);
    
    terminal_writestring("[OK] Configuring System Timer\n");
    delay(15);
    
    terminal_set_color(0x0E); // Yellow
    terminal_writestring("[INFO] Keyboard Layout: ");
    terminal_writestring(get_layout_name());
    terminal_writestring("\n");
    delay(15);
    
    terminal_set_color(0x0A); // Green
    terminal_writestring("[OK] System Ready!\n\n");
    delay(20);
    
    terminal_set_color(0x0B); // Cyan
    terminal_writestring("   _____  _                    _        ____   _____ \n");
    terminal_writestring("  / ____|| |                  | |      / __ \\ / ____|\n");
    terminal_writestring(" | (___  | |__   ___    _ __  | |__   | |  | || (___  \n");
    terminal_writestring("  \\___ \\ | '_ \\ / _ \\  | '_ \\ | '_ \\  | |  | | \\___ \\ \n");
    terminal_writestring("  ____) || | | || (_) | | |_) || |_) | | |__| | ____) |\n");
    terminal_writestring(" |_____/ |_| |_| \\___/  | .__/ |_.__/   \\____/ |_____/ \n");
    terminal_writestring("                        | |                           \n");
    terminal_writestring("                        |_|                           \n\n");
    delay(30);
    
    terminal_set_color(0x07); // Normal color
}

// Better keyboard reading with layout support
char get_keyboard_char() {
    // Check if keyboard data is available
    uint8_t status = inb(0x64);
    if (status & 0x01) {
        uint8_t scancode = inb(0x60);
        return scancode_to_ascii(scancode);
    }
    return 0;
}

// Helper function to compare strings
int str_compare(const char* str1, const char* str2, int len) {
    for (int i = 0; i < len; i++) {
        if (str1[i] != str2[i]) return 0;
    }
    return 1;
}

// Helper function to check if string starts with prefix
int starts_with(const char* str, const char* prefix, int str_len, int prefix_len) {
    if (str_len < prefix_len) return 0;
    return str_compare(str, prefix, prefix_len);
}

void draw_heart() {
    terminal_set_color(0x0C); // Bright red
    terminal_writestring("    **     **    \n");
    terminal_writestring("  ****   ****  \n");
    terminal_writestring(" ****** ****** \n");
    terminal_writestring("****************\n");
    terminal_writestring(" ************** \n");
    terminal_writestring("  ************  \n");
    terminal_writestring("   **********   \n");
    terminal_writestring("    ********    \n");
    terminal_writestring("     ******     \n");
    terminal_writestring("      ****      \n");
    terminal_writestring("       **       \n");
    terminal_set_color(0x07); // Back to normal
}

void draw_smiley() {
    terminal_set_color(0x0E); // Yellow
    terminal_writestring("    ********    \n");
    terminal_writestring("  **      **  \n");
    terminal_writestring(" *  **  **  * \n");
    terminal_writestring("*    **    *\n");
    terminal_writestring("*          *\n");
    terminal_writestring("* *      * *\n");
    terminal_writestring(" *  ****  * \n");
    terminal_writestring("  **    **  \n");
    terminal_writestring("    ********    \n");
    terminal_set_color(0x07);
}

void process_command() {
    terminal_putchar('\n');
    if (input_pos > 0) {
        input_buffer[input_pos] = '\0';
        
        if (str_compare(input_buffer, "help", input_pos) && input_pos == 4) {
            terminal_writestring("Available commands:\n");
            terminal_writestring("help - Show this help message\n");
            terminal_writestring("clear - Clear the screen\n");
            terminal_writestring("hello - Say hello!\n");
            terminal_writestring("heart - Draw a cute heart\n");
            terminal_writestring("smiley - Draw a happy face\n");
            terminal_writestring("layout - Show/change keyboard layout\n");
            terminal_writestring("echo <text> - Echo back your text\n");
            terminal_writestring("rainbow - Show pretty colors\n");
            terminal_writestring("about - About this OS\n");
            terminal_writestring("reboot - Restart the system\n");
        } 
        else if (str_compare(input_buffer, "clear", input_pos) && input_pos == 5) {
            terminal_initialize();
            terminal_writestring("Welcome to SimpleOS!\n");
            terminal_writestring("Type 'help' for available commands.\n");
        }
        else if (str_compare(input_buffer, "hello", input_pos) && input_pos == 5) {
            terminal_set_color(0x0B); // Cyan
            terminal_writestring("Hello there, wonderful programmer! <3\n");
            terminal_writestring("Your OS is working beautifully!\n");
            terminal_set_color(0x07);
        }
        else if (str_compare(input_buffer, "heart", input_pos) && input_pos == 5) {
            draw_heart();
        }
        else if (str_compare(input_buffer, "smiley", input_pos) && input_pos == 6) {
            draw_smiley();
        }
        else if (starts_with(input_buffer, "layout", input_pos, 6)) {
            if (input_pos == 6) {
                terminal_writestring("Current keyboard layout: ");
                terminal_writestring(get_layout_name());
                terminal_writestring("\n");
                terminal_writestring("Use 'layout qwerty' or 'layout qwertz' to change\n");
            } else if (starts_with(input_buffer, "layout qwerty", input_pos, 13) && input_pos == 13) {
                set_keyboard_layout(LAYOUT_QWERTY);
                terminal_writestring("Keyboard layout changed to QWERTY\n");
            } else if (starts_with(input_buffer, "layout qwertz", input_pos, 13) && input_pos == 13) {
                set_keyboard_layout(LAYOUT_QWERTZ);
                terminal_writestring("Keyboard layout changed to QWERTZ\n");
            } else {
                terminal_writestring("Usage: layout [qwerty|qwertz]\n");
            }
        }
        else if (starts_with(input_buffer, "echo ", input_pos, 5)) {
            if (input_pos > 5) {
                for (int i = 5; i < input_pos; i++) {
                    terminal_putchar(input_buffer[i]);
                }
                terminal_putchar('\n');
            } else {
                terminal_writestring("Echo: (nothing to echo)\n");
            }
        }
        else if (str_compare(input_buffer, "rainbow", input_pos) && input_pos == 7) {
            // Change colors for each letter
            uint8_t colors[] = {0x0C, 0x06, 0x0E, 0x0A, 0x0B, 0x09, 0x0D};
            char* rainbow_text = "RAINBOW";
            for (int i = 0; i < 7; i++) {
                terminal_set_color(colors[i]);
                terminal_putchar(rainbow_text[i]);
            }
            terminal_set_color(0x07);
            terminal_writestring("! <3\n");
        }
        else if (str_compare(input_buffer, "about", input_pos) && input_pos == 5) {
            terminal_set_color(0x0B); // Cyan
            terminal_writestring("SimpleOS v1.0\n");
            terminal_set_color(0x0A); // Green  
            terminal_writestring("A cute little 32-bit operating system\n");
            terminal_set_color(0x0D); // Magenta
            terminal_writestring("Made with love and lots of learning! <3\n");
            terminal_set_color(0x0E); // Yellow
            terminal_writestring("Powered by determination and curiosity\n");
            terminal_set_color(0x07);
        }
        else if (str_compare(input_buffer, "reboot", input_pos) && input_pos == 6) {
            terminal_set_color(0x0C); // Red
            terminal_writestring("Rebooting system...\n");
            delay(30);
            // Triple fault to reboot
            asm volatile ("int $0x00");
        }
        else {
            terminal_writestring("Unknown command: ");
            terminal_writestring(input_buffer);
            terminal_putchar('\n');
            terminal_writestring("Type 'help' to see available commands.\n");
        }
    }
    input_pos = 0;
    terminal_writestring("> ");
}

void kernel_main() {
    terminal_initialize();
    
    // Show boot sequence
    boot_sequence();
    
    terminal_writestring("Welcome to SimpleOS!\n");
    terminal_writestring("Advanced keyboard support with QWERTY/QWERTZ layouts.\n");
    terminal_writestring("Type 'help' for available commands.\n");
    terminal_writestring("> ");

    while (1) {
        char c = get_keyboard_char();
        if (c) {
            if (c == '\n') {
                process_command();
            } else if (c == '\b') {
                // Handle backspace
                if (input_pos > 0) {
                    input_pos--;
                    terminal_putchar('\b');
                }
            } else if (c == '\t') {
                // Ignore tabs for now
            } else if (input_pos < sizeof(input_buffer) - 1) {
                input_buffer[input_pos++] = c;
                terminal_putchar(c);
            }
        }
        
        // Small delay to prevent overwhelming the system
        for (volatile int i = 0; i < 1000; i++);
    }
}