#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stdint.h"
#include "stdbool.h"

void keyboard_handler_c();
char scancode_to_ascii(uint8_t scancode);
bool keyboard_has_input();
char keyboard_read_char();

#endif

