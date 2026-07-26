#include "cursor.h"

extern void outb(unsigned short port, unsigned char data);

static int cursor_x = 0;
static int cursor_y = 0;

static void scroll()
{
    unsigned char* vga = (unsigned char*)0xB8000;

    if (cursor_y < 25)
        return;

    for (int i = 0; i < 24 * 80 * 2; i++)
        vga[i] = vga[i + 80 * 2];

    int last_line = 24 * 80 * 2;

    for (int i = 0; i < 80 * 2; i += 2) {
        vga[last_line + i] = ' ';
        vga[last_line + i + 1] = 0x0F;
    }

    cursor_y = 24;
}

void update_cursor()
{
    unsigned short pos = cursor_y * 80 + cursor_x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, pos & 0xFF);

    outb(0x3D4, 0x0E);
    outb(0x3D5, (pos >> 8) & 0xFF);
}

void set_cursor(int x, int y)
{
    cursor_x = x;
    cursor_y = y;

    scroll();
    update_cursor();
}

void move_cursor(int dx, int dy)
{
    cursor_x += dx;
    cursor_y += dy;

    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_x < 0)
        cursor_x = 0;

    if (cursor_y < 0)
        cursor_y = 0;

    scroll();
    update_cursor();
}

void new_line()
{
    cursor_x = 0;
    cursor_y++;

    scroll();
    update_cursor();
}

int get_cursor_x(void)
{
    return cursor_x;
}

int get_cursor_y(void)
{
    return cursor_y;
}
