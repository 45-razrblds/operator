#include <stdint.h>
#include "io.h"

// Use x86 ACPI shutdown sequence
void halt_cpu(void) {
    // Disable interrupts and halt
    asm volatile ("cli; hlt");
    while(1) {}
}

void shutdown(void) {
    // Try QEMU-specific ACPI shutdown sequence
    outw(0xB004, 0x2000);  // QEMU ACPI PM1a_CNT_BLK port
    
    // Try standard ACPI shutdown sequence
    outw(0x604, 0x2000);   // Another common ACPI port
    
    // Try APM power off (port 0x47 and 0xF4)
    outb(0x47, 0x20);
    outb(0xF4, 0x00);
    
    // Try keyboard controller shutdown
    outb(0x64, 0xFE);
    
    // If nothing worked, try QEMU debug port
    const char* s = "Shutdown";
    while (*s) outb(0x8900, *s++);
    
    // If we get here, just halt the CPU
    halt_cpu();
}
