#include "stdint.h"
#include "stdbool.h"
#include "font.h"
#include "gfx.h"
#include "cursor.h"

static int16_t col = CURSOR_MARGIN;
static int16_t row = CURSOR_MARGIN;
static int16_t drawn_col = -1;
static int16_t drawn_row = -1;
static bool visible = true;

void cursor_set(int16_t new_col, int16_t new_row) {
    col = new_col;
    row = new_row;
}

void cursor_move(int16_t dcol, int16_t drow) {
    col += dcol;
    row += drow;
}

int16_t cursor_col(void) {
    return col;
}

int16_t cursor_row(void) {
    return row;
}

int32_t cursor_pixel_x(void) {
    return (int32_t)col * (int32_t)font_cell();
}

int32_t cursor_pixel_y(void) {
    return (int32_t)row * (int32_t)font_cell();
}

void cursor_newline(void) {
    cursor_set(0, row + 1);
}

void cursor_erase(void) {
    if (drawn_col < 0) return;

    uint32_t cell = font_cell();
    gfx_fill((uint32_t)drawn_col * cell, (uint32_t)drawn_row * cell, cell, cell, gfx_rgb(0, 0, 0));
    drawn_col = -1;
    drawn_row = -1;
}

void cursor_draw(void) {
    if (!visible) return;

    cursor_erase();

    uint32_t cell = font_cell();
    gfx_fill((uint32_t)cursor_pixel_x(), (uint32_t)cursor_pixel_y(), cell, cell, gfx_rgb(255, 255, 255));
    drawn_col = col;
    drawn_row = row;
}

void cursor_show(bool state) {
    if (visible == state) return;
    visible = state;

    if (state) cursor_draw();
    else cursor_erase();
}
