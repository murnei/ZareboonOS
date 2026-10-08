#ifndef CURSOR_H
#define CURSOR_H

#include "stdint.h"
#include "stdbool.h"

#define CURSOR_MARGIN 2

void cursor_set(int16_t col, int16_t row);
void cursor_move(int16_t dcol, int16_t drow);
int16_t cursor_col(void);
int16_t cursor_row(void);
int32_t cursor_pixel_x(void);
int32_t cursor_pixel_y(void);
void cursor_newline(void);
void cursor_draw(void);
void cursor_erase(void);
void cursor_show(bool visible);

#endif
