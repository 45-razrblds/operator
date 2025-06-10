#include "reboot.h"
#include "io.h"

// Use x86 keyboard controller to trigger system reset
void reboot(void) {
    uint8_t temp;
    
    // Disable interrupts
    asm volatile ("cli");
    
    // Flush keyboard controller
    do {
        temp = inb(0x64);
        if(temp & 1) inb(0x60);
    } while(temp & 2);
    
    // Try Fast A20 method first
    outb(0x92, 0x01);
    
    // Pulse CPU reset line using keyboard controller
    outb(0x64, 0xFE);
    
    // If that didn't work, try triple fault by loading invalid IDT
    uint16_t null_idt[3] = {0, 0, 0};
    asm volatile ("lidt %0; int $3" : : "m"(null_idt));
    
    // Should never get here
    while(1) {}
}
