#include "stdio.h"

void ide_wait_ready() {
    for (int i = 0; i < 1000; i++) {
        inb(0x1F7); 
    }

    while (inb(0x1F7) & 0x80);

    while (!(inb(0x1F7) & (0x40 | 0x08)));
}


void read_sector(uint32_t lba, uint8_t* buffer) {
    outb(0x1F6, (uint8_t)(0xE0 | ((lba >> 24) & 0x0F))); 
    outb(0x1F1, 0x00);
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20);

    ide_wait_ready();

    for (int i = 0; i < 256; i++) {
        uint16_t data = inw(0x1F0);
        buffer[i * 2] = (uint8_t)data;
        buffer[i * 2 + 1] = (uint8_t)(data >> 8);
    }
}

extern void outw(unsigned short port, unsigned short data);

extern void outw(unsigned short port, unsigned short data);

void write_sector(uint32_t lba, uint8_t* buffer) {
    outb(0x1F6, (uint8_t)(0xE0 | ((lba >> 24) & 0x0F)));

    ide_wait_ready();

    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x30);

    ide_wait_ready();

    for (int i = 0; i < 256; i++) {
        uint16_t data = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
        outw(0x1F0, data);
    }
    
    outb(0x1F7, 0xE7);
    ide_wait_ready();
}
