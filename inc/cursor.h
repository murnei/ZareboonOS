#ifndef CURSOR_H
#define CURSOR_H

#include "stdint.h"
#include "stdbool.h"

void move_cursor(int x, int y);
void set_cursor(int x, int y);
int16_t get_cursor_x(void);
int16_t get_cursor_y(void);
int16_t get_raw_cursor_x(void);
int16_t get_raw_cursor_y(void);
void erase_cursor();
void update_cursor();

#endif
