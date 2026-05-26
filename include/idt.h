#ifndef IDT_H
#define IDT_H

// Структура одного дескриптора (записи) в таблице
struct idt_entry {
    unsigned short base_low;
    unsigned short sel;
    unsigned char always0;
    unsigned char flags;
    unsigned short base_high;
} __attribute__((packed));

// Структура, которую понимает инструкция LIDT
struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

// Объявляем функции, чтобы их видели другие файлы
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
void idt_install();
void pic_remap();

#endif

