#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "error.h"

// Timer-Initialisierung
int timer_init(void);

// Timer-Funktionen
void delay_ms(uint32_t ms);
uint32_t get_timer_ticks(void);

// Timer-Fehlerbehandlung
enum error_code_t timer_get_last_error(void);
const char* timer_get_error_message(void);

#endif // TIMER_H
