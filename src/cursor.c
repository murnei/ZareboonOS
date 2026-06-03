#include "stdio.h"

int cursor_x = 0;
int cursor_y = 0;

void scroll() {
    unsigned char* vga = (unsigned char*)0xB8000; 

    if (cursor_y < 25) {
        return;
    }

    for (int i = 0; i < 24 * 80 * 2; i++) {
        vga[i] = vga[i + 80 * 2];
    }

    int last_line = 24 * 80 * 2;
    for (int i = 0; i < 80 * 2; i += 2) {
        vga[last_line + i] = ' ';
        vga[last_line + i + 1] = 0x0F;
    }

    cursor_y = 24;
}

void update_cursor() {
    unsigned short pos = cursor_y * 80 + cursor_x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char) ((pos >> 8) & 0xFF));
}
