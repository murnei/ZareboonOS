#include "cursor.h"

void put_char(int x, int y, char c, int color) {
     unsigned char* vga = (unsigned char*)0xB8000;
     int index = (y * 80 + x) * 2;

     vga[index] = c;
     if (!color) {
         color = 0x0F;
     }
     update_cursor();
     vga[index + 1] = color;
}

void clear() {
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            put_char(x, y, ' ', 0x00);
        }
    }

    cursor_x = 0;
    cursor_y = 0;
}
