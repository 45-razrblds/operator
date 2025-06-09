#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>
void timer_tick(void);
uint32_t get_system_ticks(void);
void delay_ms(uint32_t ms);
#endif // TIMER_H
