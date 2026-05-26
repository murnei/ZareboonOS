#include "stdio.h"
#include "system.h"

volatile char last_key_pressed = 0;
int shift_pressed = 0;
int is_extended = 0;

unsigned char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' ', 0
};

unsigned char keyboard_map_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0,
  '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' ', 0
};

unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

char get_char() {
    while (last_key_pressed == 0) {
        __asm__ volatile("sti; hlt");
    }
    char c = last_key_pressed;
    last_key_pressed = 0;
    return c;
}

void keyboard_handler_main() {
    unsigned char scancode = inb(0x60);
    __asm__ volatile("outb %%al, %%dx" : : "a"(0x20), "d"(0x20));

    if (scancode == 0xE0) {
        is_extended = 1;
        return;
    }

    if (is_extended) {
        is_extended = 0;
        if (!(scancode & 0x80)) {
            if (scancode == 0x48) last_key_pressed = 1;
            if (scancode == 0x50) last_key_pressed = 2;
            if (scancode == 0x4B) last_key_pressed = 3;
            if (scancode == 0x4D) last_key_pressed = 4;
        }
        return;
    }

    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return;
    }

    if (!(scancode & 0x80)) {
        char letter = shift_pressed ? keyboard_map_shift[scancode] : keyboard_map[scancode];
        if (letter != 0) {
            last_key_pressed = letter;
        }
    }
}
