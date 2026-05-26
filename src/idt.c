struct idt_entry {
    unsigned short base_low;
    unsigned short sel;
    unsigned char always0;
    unsigned char flags;
    unsigned short base_high;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

extern void idt_load();

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void pic_remap() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // Смещаем IRQ0-7 на 32-39
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0xFD);
    outb(0xA1, 0xFF);
    outb(0x21, 0xFC);                                 outb(0xA1, 0xFF);
}

void idt_install() {
    extern void irq0_handler();
    extern void irq1_handler();
    extern void syscall_handler_asm();
    
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;

    for(int i = 0; i < 256; i++) idt_set_gate(i, 0, 0, 0);

    idt_set_gate(32, (unsigned long)irq0_handler, 0x08, 0x8E);
    idt_set_gate(33, (unsigned long)irq1_handler, 0x08, 0x8E);
    idt_set_gate(0x80, (unsigned long)syscall_handler_asm, 0x08, 0x8F);
    
    pic_remap();
    idt_load();

    __asm__ volatile("sti");
}
