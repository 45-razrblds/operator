#include "timer.h"
#include "error.h"
#include "io.h"

#define PIT_FREQUENCY 1193180
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43

static uint32_t timer_ticks = 0;
static enum error_code_t last_error = ERR_NONE;

int timer_init(void) {
    // Konfiguriere PIT für 1ms Interrupts
    uint32_t divisor = PIT_FREQUENCY / 1000;
    
    // Überprüfe PIT-Zugriff
    if (!io_port_available(PIT_COMMAND)) {
        ERROR_SET(ERR_DEVICE_NOT_FOUND, "PIT not accessible");
        return 0;
    }
    
    // Setze PIT-Modus
    outb(PIT_COMMAND, 0x36);  // Channel 0, Mode 3, 16-bit
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
    
    timer_ticks = 0;
    last_error = ERR_NONE;
    return 1;
}

void timer_tick(void) {
    timer_ticks++;
}

uint32_t get_timer_ticks(void) {
    return timer_ticks;
}

void delay_ms(uint32_t ms) {
    uint32_t start = timer_ticks;
    while (timer_ticks - start < ms) {
        // Warte auf nächsten Tick
        asm volatile("hlt");
    }
}

enum error_code_t timer_get_last_error(void) {
    return last_error;
}

const char* timer_get_error_message(void) {
    return error_to_string(last_error);
}
