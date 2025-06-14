#ifndef COMMAND_H
#define COMMAND_H

#include <stdint.h>
#include "error.h"

// Command-Initialisierung
int command_init(void);

// Command-Verarbeitung
void command_process_input(uint16_t key);

// Command-Ausführung
void command_execute(const char* command);

// Command-Registrierung
void command_register(const char* name, void (*handler)(void));
void command_register_hidden(const char* name, void (*handler)(void));

extern char input_buffer[256];

#endif // COMMAND_H
