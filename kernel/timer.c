#include "timer.h"
#include <stdint.h>
volatile uint32_t timer_ticks = 0;
void timer_tick() { timer_ticks++; }
uint32_t get_system_ticks() { return timer_ticks; }
void delay_ms(uint32_t ms) {
    uint32_t start = get_system_ticks();
    while ((get_system_ticks() - start) < (ms / 10)) {
        timer_tick();
    }
}
