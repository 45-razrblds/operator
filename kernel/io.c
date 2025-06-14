#include "io.h"
#include "error.h"

// I/O-Port-Verfügbarkeit prüfen
int io_port_available(uint16_t port) {
    // In einer realen Implementierung würden wir hier prüfen,
    // ob der Port tatsächlich verfügbar ist. Für jetzt
    // nehmen wir an, dass alle Ports verfügbar sind.
    return 1;
}

// Read a byte from the specified port
uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Write a byte to the specified port
void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Read a 16-bit value from the specified port
uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Write a 16-bit value to the specified port
void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

// Read a 32-bit value from the specified port
uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Write a 32-bit value to the specified port
void outl(uint16_t port, uint32_t val) {
    __asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}
