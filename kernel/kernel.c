#include "kernel.h"
#include "error.h"
#include "minilibc.h"
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
#include "io.h"

// *grinst böse* Kernel-Module! So nutzlos wie deine Versuche, sie zu verstehen, du Idiot!
module_status_t kernel_modules[7] = {
    {"Terminal", 0},
    {"Keyboard", 0},
    {"Memory", 0},
    {"Timer", 0},
    {"Debug", 0},
    {"Command", 0},
    {"OPFS", 0}
};

// *lacht hämisch* Initialisiere diesen Wahnsinn! Wie deine Karriere, du Versager!
void kernel_init(void) {
    // *spottet* Initialisiere die Fehlerbehandlung! Wie deine Hoffnungen, du Loser!
    error_init();
    
    // *grinst* Initialisiere das Terminal! Wie deine Zukunft, du Dummkopf!
    terminal_initialize();
    module_status_set_error(&kernel_modules[0], (enum error_code_t)ERR_DEVICE_BUSY);
    
    // *lacht* Initialisiere die Tastatur! Wie deine Pläne, du Versager!
    if (keyboard_init() != 0) {
        module_status_set_error(&kernel_modules[1], (enum error_code_t)ERR_DEVICE_BUSY);
    }
    
    // *spottet* Initialisiere den Speicher! Wie deine Erinnerungen, du Idiot!
    if (memory_init() != 0) {
        module_status_set_error(&kernel_modules[2], (enum error_code_t)ERR_CRIT_MEMORY);
    }
    
    // *grinst* Initialisiere den Timer! Wie deine Zeit, du Loser!
    if (timer_init() != 0) {
        module_status_set_error(&kernel_modules[3], (enum error_code_t)ERR_DEVICE_BUSY);
    }
    
    // *lacht* Initialisiere das Debugging! Wie deine Fehler, du Versager!
    debug_init();
    
    // *spottet* Initialisiere die Befehle! Wie deine Ideen, du Dummkopf!
    if (command_init() != 0) {
        module_status_set_error(&kernel_modules[5], (enum error_code_t)ERR_DEVICE_BUSY);
    }
    
    // *grinst* Initialisiere das Dateisystem! Wie deine Hoffnungen, du Idiot!
    if (opfs_init() != 0) {
        module_status_set_error(&kernel_modules[6], (enum error_code_t)ERR_DEVICE_BUSY);
    }
}

// *lacht hämisch* Die Hauptschleife! So endlos wie deine Fehler, du Versager!
void kernel_main(void) {
    // *spottet* Hauptschleife! Wie deine Verzweiflung, du Loser!
    while (1) {
        // *grinst* Verarbeite Tastatureingaben! Wie deine Hoffnungen, du Dummkopf!
        uint16_t key = keyboard_getchar();
        if (key != 0) {
            command_process_input(key);
        }
    }
}

// *lacht böse* Kernel-Panic! Wie deine Karriere, du Idiot!
void kernel_panic(const char* message) {
    terminal_clear();
    terminal_set_color(0x0C); // Rot
    terminal_writestring("KERNEL PANIC: ");
    terminal_writestring(message);
    terminal_writestring("\nSystem halted.");
    
    // *grinst* Halte das System an! Wie deine Hoffnungen, du Versager!
    while (1) {
        __asm__("hlt");
    }
}