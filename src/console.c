#include "font.h"
#include "video.h"
#include "stdint.h"
#include "cursor.h"
#include "stdio.h"

#define SCREEN_MARGIN_X 2

static uint8_t global_scale = 2;

uint8_t get_font_scale() {
    return global_scale;
}

void set_font_scale(uint8_t scale) {
    global_scale = scale;
}

void new_line() {
    set_cursor(SCREEN_MARGIN_X, get_cursor_y() + 1);
}

void draw_char(int16_t start_x, int16_t start_y, uint8_t c, uint32_t color) {
    if (global_scale <= 0) { global_scale = 2; }

    uint8_t* vga_c = (uint8_t*)get_font(c);
    if (!vga_c) return;

    int scale = global_scale;
    int glyph_size = 8 * scale;
    if (glyph_size < 1) glyph_size = 1;

    for (int py = 0; py < glyph_size; py++) {
        int src_y = py / scale;
        if (src_y > 7) src_y = 7;

        unsigned char row = vga_c[src_y];

        for (int px = 0; px < glyph_size; px++) {
            int src_x = px / scale;
            if (src_x > 7) src_x = 7;

            if ((row & (0x80 >> src_x)) != 0) {
                put_pixel(start_x + px, start_y + py, color);
            }
        }
    }
}

void print_char(uint8_t ch, uint32_t color) {
    if (ch == '\n') {
        new_line();
        return;
    } else if (ch == '\t') {
        move_cursor(4, 0);
    }

    if (get_cursor_x() < SCREEN_MARGIN_X) {
        set_cursor(SCREEN_MARGIN_X, get_cursor_y());
    }

    if (get_cursor_x() >= SCREEN_WIDTH) {
        set_cursor(SCREEN_MARGIN_X, get_cursor_y() + 1);
    }

    if (get_cursor_x() >= SCREEN_HEIGHT) {
        set_cursor(SCREEN_MARGIN_X, 0);
    }

    draw_char(get_raw_cursor_x(), get_raw_cursor_y(), ch, color);
    move_cursor(1, 0);
}

void clear() {
    video_memset(0, 0, rgb(0, 0, 0), (uint32_t)SCREEN_WIDTH * SCREEN_HEIGHT);

    set_cursor(0, 0);
}

uint32_t atoi(const uint8_t* str) {
    uint32_t res = 0;

    while (*str >= '0' && *str <= '9') {
        res = res * 10 + (*str - '0');
        str++;
    }

    return res;
}
