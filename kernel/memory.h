#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

void* malloc(size_t size);
void free(void* ptr);
size_t get_heap_used(void);
size_t get_heap_total(void);
int test_memory(void);

#endif // MEMORY_H
