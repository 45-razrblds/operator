#include "memory.h"
#include <stddef.h>
#include <stdint.h>
#define HEAP_START 0x1000000
#define HEAP_SIZE  0x100000
static uintptr_t heap_ptr = HEAP_START;
void* malloc(size_t size) {
    void* ptr = (void*)heap_ptr;
    heap_ptr += size;
    if (heap_ptr > HEAP_START + HEAP_SIZE) {
        return 0;
    }
    return ptr;
}
size_t get_heap_used(void) {
    return (size_t)(heap_ptr - HEAP_START);
}
size_t get_heap_total(void) {
    return HEAP_SIZE;
}
int test_memory() {
    char* ptr = (char*)malloc(16);
    if (!ptr) return 0;
    for (int i = 0; i < 16; i++) ptr[i] = (char)i;
    for (int i = 0; i < 16; i++) {
        if (ptr[i] != (char)i) return 0;
    }
    return 1;
}
