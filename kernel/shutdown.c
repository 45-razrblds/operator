#include <stdint.h>
#include "io.h"

// Use x86 ACPI shutdown sequence
void halt_cpu(void) {
    // Disable interrupts and halt
    asm volatile ("cli; hlt");
    while(1) {}
}

void shutdown(void) {
    // Try ACPI shutdown first
    // Port 0x604 is common for ACPI shutdown on emulators like QEMU
    // Write 0x2000 to shutdown
    outb(0x604, 0x02);
    
    // If that didn't work, try another common QEMU method
    // Write "Shutdown" to the QEMU debug port
    const char* s = "Shutdown";
    while (*s) outb(0x8900, *s++);
    
    // If we get here, just halt the CPU
    halt_cpu();
}
