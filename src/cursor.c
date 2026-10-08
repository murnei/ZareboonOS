#include "stdint.h"
#include "stdbool.h"
#include "console.h"
#include "video.h"
#include "time.h"

static int16_t cursor_x = 2;
static int16_t cursor_y = 2;
static int16_t drawn_x = -1;
static int16_t drawn_y = -1;

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

void erase_cursor(void) {
    if (drawn_x < 0) return;
    int g = 8 * get_font_scale();
    fill_rect(drawn_x * g, drawn_y * g, g, g, rgb(0, 0, 0));
    drawn_x = drawn_y = -1;
}

void update_cursor(void) {
    erase_cursor();
    int g = 8 * get_font_scale();
    fill_rect(get_raw_cursor_x(), get_raw_cursor_y(), g, g, rgb(255, 255, 255));
    drawn_x = cursor_x;
    drawn_y = cursor_y;
}
