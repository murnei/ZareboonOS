#ifndef CONSOLE_H
#define CONSOLE_H

#include "stdint.h"

int get_font_scale();
void set_font_scale(uint8_t scale);

void draw_char(int16_t start_x, int16_t start_y, uint8_t c, uint32_t color);
void new_line();
void clear();
void print_char(uint8_t ch, uint32_t color);
uint32_t atoi(const uint8_t* str);

#endif
