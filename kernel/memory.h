#ifndef MEMORY_H
#define MEMORY_H
#include <stddef.h>
void* malloc(size_t size);
int test_memory(void);
size_t get_heap_used(void);
size_t get_heap_total(void);
#endif // MEMORY_H
