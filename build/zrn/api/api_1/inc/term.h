#ifndef TERM_H
#define TERM_H

#include "stdint.h"

void term_putc(char ch, uint32_t color);
void term_write(const char* text, uint32_t color);
void term_clear(void);

#endif
