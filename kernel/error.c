#include "error.h"
#include "terminal.h"
#include "minilibc.h"

// uwu~ Current error state
static enum error_code_t current_error = ERR_NONE;

// Error-Buffer für Formatierung
static char error_buffer[256];

// Terminal-Farbe speichern
static uint8_t current_terminal_color = 0x07; // Standardfarbe

// Globale Fehlervariable
int last_error = 0;

// Fehlermeldungen
const char* error_messages[] = {
    "No error",
    "Invalid argument",
    "Operation not permitted",
    "No such file or directory",
    "Device or resource busy",
    "Out of memory",
    "Invalid operation",
    "Unknown error",
    "Device not found",
    "Critical memory error"
};

// Terminal-Farbe abrufen
uint8_t terminal_get_color(void) {
    return current_terminal_color;
}

// kawaii Error functions :3
void error_set(enum error_code_t code) {
    current_error = code;
    
    // Print error message uwu~
    if (code != ERR_NONE) {
        terminal_set_color(0x0C); // Red >w<
        terminal_writestring("ERROR: ");
        terminal_writestring(error_to_string(code));
        terminal_writestring("\n");
        terminal_set_color(0x07); // Back to normal color :3
    }
}

// Error loggen
void error_log(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(error_buffer, sizeof(error_buffer), format, args);
    va_end(args);
    
    terminal_writestring(error_buffer);
    terminal_writestring("\n");
}

// Error-Status abrufen
enum error_code_t error_get(void) {
    return current_error;
}

// Error-Nachricht abrufen
const char* error_get_message(void) {
    return error_to_string(current_error);
}

// Error-Datei abrufen
const char* error_get_file(void) {
    return NULL; // This function is not provided in the new implementation
}

// Error-Zeile abrufen
int error_get_line(void) {
    return 0; // This function is not provided in the new implementation
}

// Prüfen ob Error kritisch ist
int error_is_critical(enum error_code_t code) {
    return (code >= ERR_CRIT_START && code <= ERR_CRIT_END);
}

void error_init(void) {
    current_error = ERR_NONE;
    current_terminal_color = 0x07; // Standardfarbe
    last_error = 0;
}

void error_clear(void) {
    current_error = ERR_NONE;
}

// uwu~ Helper function to convert error codes to strings :3
const char* error_to_string(enum error_code_t code) {
    if (code >= 0 && code < sizeof(error_messages) / sizeof(error_messages[0])) {
        return error_messages[code];
    }
    return "Unknown error";
}

// Modul-Status-Implementierung
void module_status_init(module_status_t* module, const char* name) {
    module->name = name;
    module->status = 0;
}

void module_status_set_error(module_status_t* module, enum error_code_t code) {
    if (module != NULL) {
        module->status = code;
        error_set(code);
    }
}

void module_status_clear(module_status_t* module) {
    module->status = 0;
}

int module_status_is_healthy(module_status_t* module) {
    return module->status;
}

// Fehler abrufen
int error_get_global(void) {
    return last_error;
}

// Fehler setzen
void error_set_global(int error) {
    last_error = error;
}

// Fehlermeldung ausgeben
void error_print(void) {
    if (current_error != ERR_NONE) {
        terminal_set_color(0x0C); // Red uwu~
        terminal_writestring("Current error: ");
        terminal_writestring(error_to_string(current_error));
        terminal_writestring("\n");
        terminal_set_color(0x07); // Back to normal color >w<
    }
} 