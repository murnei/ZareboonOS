#ifndef SYSTEM_H
#define SYSTEM_H

void reboot();
void sleep(unsigned long ticks_to_wait);
void init_timer(uint32_t frequency);
void display_time();

#endif
