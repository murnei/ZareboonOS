#include "stdint.h"

#include "stdio.h"
#include "io.h"

#define TIMER_FREQUENCY 100

volatile uint32_t timer_ticks = 0;

uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd & 0xF0) >> 4) * 10 + ((bcd & 0x0F));
}

struct time_now {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
};

void get_time(struct time_now* t) {
    outb(0x70, 0x00);
    t->seconds = bcd_to_bin(inb(0x71));
    outb(0x70, 0x02);
    t->minutes = bcd_to_bin(inb(0x71));
    outb(0x70, 0x04);
    t->hours = bcd_to_bin(inb(0x71));
}

void timer_handler_c() {
    timer_ticks++;

    outb(0x20, 0x20);
}

void sleep(const uint32_t ms) {
    if (ms == 0) return;
    uint32_t ticks_to_wait = (ms * TIMER_FREQUENCY) / 1000;
  
    if (ticks_to_wait == 0) ticks_to_wait = 1;

    uint32_t start_ticks = timer_ticks;

	while (1) {
        volatile uint32_t current_ticks = timer_ticks;
        if ((current_ticks - start_ticks) >= ticks_to_wait) {
            break;
        }
    }
}
