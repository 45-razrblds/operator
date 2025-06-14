#include "command.h"
#include "error.h"
#include "minilibc.h"
#include "terminal.h"
#include "shutdown.h"
#include "reboot.h"
#include "memory.h"
#include "timer.h"
#include "debug.h"
#include "opfs.h"
#include "boot.h"
#include "serial.h"

// Command-Buffer
char input_buffer[256];
static int input_pos = 0;

// Command-Handler
typedef void (*command_handler_t)(void);
static struct {
    const char* name;
    command_handler_t handler;
    int hidden;
} commands[32];
static int command_count = 0;

// Command-Registrierung
void command_register(const char* name, command_handler_t handler) {
    if (command_count < 32) {
        commands[command_count].name = name;
        commands[command_count].handler = handler;
        commands[command_count].hidden = 0;
        command_count++;
    }
}

void command_register_hidden(const char* name, command_handler_t handler) {
    if (command_count < 32) {
        commands[command_count].name = name;
        commands[command_count].handler = handler;
        commands[command_count].hidden = 1;
        command_count++;
    }
}

// Command-Verarbeitung
void command_process_input(uint16_t key) {
    if (key == '\b') {
        if (input_pos > 0) {
            input_pos--;
            terminal_putchar('\b');
            terminal_putchar(' ');
            terminal_putchar('\b');
        }
    } else if (key == '\n') {
        terminal_putchar('\n');
        input_buffer[input_pos] = '\0';
        command_execute(input_buffer);
        input_pos = 0;
        terminal_writestring("> ");
    } else if (input_pos < 255) {
        input_buffer[input_pos++] = key;
        terminal_putchar(key);
    }
}

// Command-Ausführung
void command_execute(const char* command) {
    if (strcmp(command, "help") == 0) {
        terminal_writestring("Available commands:\n");
        for (int i = 0; i < command_count; i++) {
            if (!commands[i].hidden) {
                terminal_writestring("  ");
                terminal_writestring(commands[i].name);
                terminal_writestring("\n");
            }
        }
    } else if (strcmp(command, "clear") == 0) {
        terminal_clear();
    } else if (strcmp(command, "shutdown") == 0) {
        shutdown();
    } else if (strcmp(command, "reboot") == 0) {
        reboot();
    } else if (strcmp(command, "memory") == 0) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Memory used: %d/%d bytes\n", 
                get_heap_used(), get_heap_total());
        terminal_writestring(buf);
    } else if (strcmp(command, "debug") == 0) {
        debug_mode = !debug_mode;
        terminal_writestring(debug_mode ? "Debug mode enabled\n" : "Debug mode disabled\n");
    } else if (strncmp(command, "cd ", 3) == 0) {
        opfs_cd(command + 3);
    } else if (strncmp(command, "ls", 2) == 0) {
        opfs_ls(NULL);  // NULL bedeutet aktuelles Verzeichnis
    } else if (strncmp(command, "mkdir ", 6) == 0) {
        opfs_mkdir(command + 6);
    } else if (strncmp(command, "touch ", 6) == 0) {
        opfs_touch(command + 6);
    } else if (strncmp(command, "cat ", 4) == 0) {
        opfs_cat(command + 4);
    } else if (strncmp(command, "echo ", 5) == 0) {
        const char* space = strchr(command + 5, ' ');
        if (space) {
            char path[256];
            int len = space - (command + 5);
            strncpy(path, command + 5, len);
            path[len] = '\0';
            // Verwende opfs_touch und opfs_cat statt opfs_echo
            opfs_touch(path);
            opfs_cat(path);
        } else {
            terminal_writestring("Usage: echo <path> <content>\n");
        }
    } else {
        terminal_writestring("Unknown command. Type 'help'.\n");
    }
}

// Command-Initialisierung
int command_init(void) {
    input_pos = 0;
    command_count = 0;
    
    // Registriere Standard-Befehle
    command_register("help", NULL);
    command_register("clear", NULL);
    command_register("shutdown", NULL);
    command_register("reboot", NULL);
    command_register("memory", NULL);
    command_register("debug", NULL);
    command_register("cd", NULL);
    command_register("ls", NULL);
    command_register("mkdir", NULL);
    command_register("touch", NULL);
    command_register("cat", NULL);
    command_register("echo", NULL);
    
    return 1;
}
