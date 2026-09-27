#include "stdint.h"
#include "io.h"

struct idt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_ptr_struct idt_ptr;
extern void timer_isr_stub();

__attribute__((aligned(16))) uint8_t my_idt_table[256 * 8];

void init_timer(uint32_t frequincy) {
    if (frequincy <= 0) return;
    uint32_t divisor = 1193182 / frequincy;

    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

void set_idt_gate(uint8_t vector, uint32_t handler_address) {
    uint32_t offset = vector * 8;
    uint16_t low  = (uint16_t)(handler_address & 0xFFFF);
    uint16_t high = (uint16_t)((handler_address >> 16) & 0xFFFF);

    *(uint16_t*)&my_idt_table[offset + 0] = low;
    *(uint16_t*)&my_idt_table[offset + 2] = 0x0008;
    *(uint8_t*)&my_idt_table[offset + 4]  = 0x00;
    *(uint8_t*)&my_idt_table[offset + 5]  = 0x8E;
    *(uint16_t*)&my_idt_table[offset + 6] = high;
}

void set_idt_gate_user(uint8_t vector, uint32_t handler_address) {
    uint32_t offset = vector * 8;
    uint16_t low  = (uint16_t)(handler_address & 0xFFFF);
    uint16_t high = (uint16_t)((handler_address >> 16) & 0xFFFF);

    *(uint16_t*)&my_idt_table[offset + 0] = low;
    *(uint16_t*)&my_idt_table[offset + 2] = 0x0008;
    *(uint8_t*)&my_idt_table[offset + 4]  = 0x00;
    *(uint8_t*)&my_idt_table[offset + 5]  = 0xEE;
    *(uint16_t*)&my_idt_table[offset + 6] = high;
}

extern void* isr_stub_table[];
extern void keyboard_isr_stub();

void pic_remap() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);
}

void init_idt() {
    idt_ptr.limit = (256 * 8) - 1;
    idt_ptr.base  = (uint32_t)&my_idt_table;

    for (int i = 0; i < 256; i++) {
        *(uint64_t*)&my_idt_table[i * 8] = 0;
    }

    for (int i = 0; i < 32; i++) {
        set_idt_gate(i, (uint32_t)isr_stub_table[i]);
    }
    
    set_idt_gate(32, (uint32_t)timer_isr_stub);
    set_idt_gate(33, (uint32_t)keyboard_isr_stub);

    pic_remap();

    init_timer(100);

    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
    __asm__ volatile("sti");
}

void isr_handler_c() {
    while(1);
}
