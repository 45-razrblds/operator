#include "memory.h"
#include "error.h"
#include "terminal.h"
#include <stddef.h>
#include <stdint.h>
#define HEAP_START 0x1000000
#define HEAP_SIZE  0x100000
static uintptr_t heap_ptr = HEAP_START;
static uint8_t heap[HEAP_SIZE];
static size_t heap_used = 0;
static enum error_code_t last_error = ERR_NONE;
static void* heap_start = NULL;
static void* heap_end = NULL;

// *grinst böse* Ah, die Speicherverwaltung! Ein weiteres Meisterwerk deines Versagens, du Idiot!
int memory_init(void) {
    heap_used = 0;
    last_error = ERR_NONE;
    
    // Überprüfe Heap-Zugriff
    if (heap == NULL) {
        ERROR_SET(ERR_CRIT_MEMORY, "Heap memory not accessible");
        return -1;
    }
    
    // *lacht hämisch* Initialisiere diesen Wahnsinn! Wie deine Karriere, du Versager!
    heap_start = (void*)0x100000;  // 1MB
    heap_end = (void*)0x200000;    // 2MB
    
    if (heap_start == NULL || heap_end == NULL) {
        ERROR_SET(ERR_CRIT_MEMORY, "*grinst* Der Heap lebt nicht! Wie deine Hoffnungen, du Idiot!");
        return -1;
    }
    
    return 0;
}

// *lacht* Allokiere diesen Speicher! Wie deine Zeit, du Versager!
void* kmalloc(size_t size) {
    if (size == 0) {
        ERROR_SET(ERR_INVALID_PARAMETER, "*spottet* Null Bytes? Was soll ich damit? Ich bin kein Zauberer, du Dummkopf!");
        return NULL;
    }
    
    if (heap_used + size > HEAP_SIZE) {
        ERROR_SET(ERR_CRIT_MEMORY, "Out of memory");
        return NULL;
    }
    
    void* ptr = &heap[heap_used];
    heap_used += size;
    return ptr;
}

// *lacht hämisch* Gib den Speicher frei! Er muss sterben! Wie deine Träume, du Versager!
void kfree(void* ptr) {
    if (ptr == NULL) {
        return;  // *spottet* Null... wie deine Zukunft, du Idiot...
    }
    
    // *grinst* TODO: Implementiere die Freigabe! Wie deine Karriere, du Loser!
    ERROR_SET(ERR_WARN_NOT_IMPLEMENTED, "*lacht* Die Speicherfreigabe... sie lebt noch nicht... wie deine Hoffnungen, du Versager!");
}

size_t get_heap_used(void) {
    return heap_used;
}

size_t get_heap_total(void) {
    return HEAP_SIZE;
}

// *spottet* Sag mir, was schiefgelaufen ist! Wie immer bei dir, du Dummkopf!
enum error_code_t memory_get_last_error(void) {
    return last_error;
}

// *grinst* Übersetze es! In die Sprache des Scheiterns, du Idiot!
const char* memory_get_error_message(void) {
    return error_to_string(last_error);
}

int test_memory(void) {
    char* ptr = (char*)kmalloc(16);
    if (!ptr) {
        ERROR_SET(ERR_CRIT_MEMORY, "Memory allocation failed in test");
        return 0;
    }
    
    // Schreibe Testdaten
    for (int i = 0; i < 16; i++) {
        ptr[i] = (char)i;
    }
    
    // Überprüfe Testdaten
    for (int i = 0; i < 16; i++) {
        if (ptr[i] != (char)i) {
            ERROR_SET(ERR_CRIT_MEMORY, "Memory test failed: data corruption");
            return 0;
        }
    }
    
    return 1;
}
