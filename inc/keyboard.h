#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stdint.h"
#include "stdbool.h"

enum {
    KEY_UP = 0x80,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_HOME,
    KEY_END,
    KEY_DELETE
};

bool keyboard_has_input(void);
char keyboard_read_char(void);
char scancode_to_ascii(uint8_t scancode);
void keyboard_handler_c(void);

#endif
