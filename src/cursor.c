#include "stdint.h"
#include "console.h"
#include "video.h"

static int16_t cursor_x = 2;
static int16_t cursor_y = 2;

void set_cursor(int16_t x, int16_t y) {
    cursor_x = x;
    cursor_y = y;
}

void move_cursor(int16_t x, int16_t y) {
    if (x == 0 && y == 0) {
        return;
    }

    cursor_x += x;
    cursor_y += y;
}

int16_t get_cursor_x(void) {
    return cursor_x;
}

int16_t get_cursor_y(void) {
    return cursor_y;
}

int16_t get_raw_cursor_x(void) {
    return cursor_x * (8 * get_font_scale());
}

int16_t get_raw_cursor_y(void) {
    return cursor_y * (8 * get_font_scale());
}
