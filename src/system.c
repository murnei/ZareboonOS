#include "stdio.h"

extern unsigned char inb(unsigned short port);
extern void outb(unsigned short port, unsigned char data);

void reboot() {
    unsigned char good = 0x02;
    
    while (good & 0x02) {
        good = inb(0x64);
    }
    
    outb(0x64, 0xFE);
}

volatile unsigned long timer_ticks = 0;
void timer_handler() {
    timer_ticks++;
    outb(0x20, 0x20);
}


unsigned char get_rtc_register(int reg) {
    outb(0x70, reg);
    return inb(0x71);
}

unsigned char bcd_to_bin(unsigned char bcd) {
    return ((bcd / 16) * 10) + (bcd % 16);
}

void get_time(char* buffer) {
    unsigned char second = bcd_to_bin(get_rtc_register(0x00));
    unsigned char minute = bcd_to_bin(get_rtc_register(0x02));
    unsigned char hour   = bcd_to_bin(get_rtc_register(0x04));
}

void init_timer(unsigned int frequency) {
    unsigned int divisor = 1193180 / frequency;
    outb(0x43, 0x36);
    outb(0x40, (unsigned char)(divisor & 0xFF));
    outb(0x40, (unsigned char)((divisor >> 8) & 0xFF));
    
    unsigned char mask = inb(0x21);
    outb(0x21, mask & ~0x01); 
}

void sleep(unsigned long ticks_to_wait) {
    unsigned long end = timer_ticks + ticks_to_wait;
    while (timer_ticks < end) {
        __asm__ volatile("nop"); 
    }
}

void display_time() {
    print("\n");
    print_int(bcd_to_bin(get_rtc_register(0x04)));
    print(":");
    print_int(bcd_to_bin(get_rtc_register(0x02)));
    print(":");
    print_int(bcd_to_bin(get_rtc_register(0x00)));
}
