#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include "error.h"

// Memory-Initialisierung
int memory_init(void);

// Memory-Allokation
void* kmalloc(size_t size);
void kfree(void* ptr);

// Memory-Status
size_t get_heap_used(void);
size_t get_heap_total(void);

// Memory-Tests
int test_memory(void);

// Memory-Fehlerbehandlung
enum error_code_t memory_get_last_error(void);
const char* memory_get_error_message(void);

#endif // MEMORY_H
