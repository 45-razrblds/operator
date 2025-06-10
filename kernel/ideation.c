#include "terminal.h"
#include "minilibc.h"

void ideation_command(void) {
    const char* msg = "Shoutout to Team Nexus for being cool people <3";
    int len = strlen(msg); // Use actual string length
    uint8_t start_r = 0xa2, start_g = 0x59, start_b = 0xff;
    uint8_t end_r = 0x00, end_g = 0xff, end_b = 0xf0;
    for (int i = 0; i < len; i++) {
        float t = (float)i / (len - 1);
        uint8_t r = (uint8_t)(start_r + t * (end_r - start_r));
        uint8_t g = (uint8_t)(start_g + t * (end_g - start_g));
        uint8_t b = (uint8_t)(start_b + t * (end_b - start_b));
        terminal_setcolor_rgb(r, g, b);
        terminal_putchar(msg[i]);
    }
    terminal_setcolor_default();
    terminal_putchar('\n');
}
