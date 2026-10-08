#ifndef FONT_H
#define FONT_H

#include "stdint.h"

#define FONT_SIZE 8

uint8_t font_scale(void);
void font_set_scale(uint8_t scale);
uint32_t font_cell(void);
const uint8_t* font_glyph(uint8_t ch);

#endif
