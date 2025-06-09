#include "command.h"
#include "terminal.h"
#include "debug.h"
#include "opfs.h"
#include "shutdown.h"
#include "reboot.h"
#include "minilibc.h"
#include "serial.h"

char input_buffer[256];
int input_pos = 0;
extern int debug_mode;

// Helper: trim leading/trailing whitespace and newlines in-place
static void trim_input(char* buf) {
    // Trim leading
    char* p = buf;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (p != buf) {
        char* d = buf;
        while (*p) *d++ = *p++;
        *d = 0;
    }
    // Trim trailing
    int len = strlen(buf);
    while (len > 0 && (buf[len-1] == ' ' || buf[len-1] == '\t' || buf[len-1] == '\r' || buf[len-1] == '\n'))
        buf[--len] = 0;
}

void process_command() {
    serial_writestring("[SERIAL] process_command() entry\n");
    serial_writestring("[SERIAL] input_buffer: ");
    serial_writestring(input_buffer);
    serial_writestring("\n");
    
    trim_input(input_buffer);
    
    serial_writestring("[SERIAL] input_buffer after trim: ");
    serial_writestring(input_buffer);
    serial_writestring("\n");

    if (strncmp(input_buffer, "ls", 2) == 0) {
        char* arg = input_buffer+2; while (*arg == ' ') arg++;
        opfs_ls(*arg ? arg : NULL);
    } else if (strncmp(input_buffer, "cat ", 4) == 0) {
        opfs_cat(input_buffer+4);
    } else if (strncmp(input_buffer, "edit ", 5) == 0) {
        opfs_edit(input_buffer+5);
    } else if (strncmp(input_buffer, "touch ", 6) == 0) {
        opfs_touch(input_buffer+6);
    } else if (strncmp(input_buffer, "rm ", 3) == 0) {
        opfs_rm(input_buffer+3);
    } else if (strncmp(input_buffer, "mkdir ", 6) == 0) {
        opfs_mkdir(input_buffer+6);
    } else if (strncmp(input_buffer, "rmdir ", 6) == 0) {
        opfs_rm(input_buffer+6);
    } else if (strncmp(input_buffer, "cd ", 3) == 0) {
        opfs_cd(input_buffer+3);
    } else if (strncmp(input_buffer, "pwd", 3) == 0) {
        opfs_pwd();
    } else if (strncmp(input_buffer, "debug", 5) == 0) {
        toggle_debug_mode();
    } else if (strncmp(input_buffer, "shutdown", 8) == 0) {
        terminal_set_color(0x0C);
        terminal_writestring("\nShutting down...\n");
        terminal_set_color(0x07);
        // delay(100); // Optionally add delay
        shutdown();
    } else if (strncmp(input_buffer, "reboot", 6) == 0) {
        terminal_set_color(0x0C);
        terminal_writestring("\nRebooting...\n");
        terminal_set_color(0x07);
        // delay(100); // Optionally add delay
        reboot();
    } else if (strncmp(input_buffer, "help", 4) == 0) {
        terminal_set_color(0x0F);
        terminal_writestring("Available commands:\n");
        terminal_set_color(0x07);
        terminal_writestring("  ls [dir]       - List files and directories\n");
        terminal_writestring("  cat <file>     - Display file contents\n");
        terminal_writestring("  edit <file>    - Edit file contents (ESC to save and exit)\n");
        terminal_writestring("  touch <file>   - Create a new empty file\n");
        terminal_writestring("  rm <file|dir>  - Remove a file or directory\n");
        terminal_writestring("  mkdir <dir>    - Create a new directory\n");
        terminal_writestring("  cd <dir>       - Change current directory\n");
        terminal_writestring("  pwd            - Print working directory\n");
        terminal_writestring("  clear          - Clear the screen\n");
        terminal_writestring("  debug          - Toggle debug mode\n");
        terminal_writestring("  shutdown       - Power off the system\n");
        terminal_writestring("  reboot         - Restart the system\n");
        terminal_writestring("  help           - Show this help message\n");
    } else if (strncmp(input_buffer, "clear", 5) == 0) {
        terminal_initialize();
        // draw_banner();
    } else {
        terminal_writestring("Unknown command. Type 'help'.\n");
    }
    input_pos = 0;
    memset(input_buffer, 0, 256);
    if (debug_mode) draw_debug_info();
}
