#include "stdint.h"
#include "stdbool.h"
#include "zrn_syscall.h"
#include "font.h"
#include "cursor.h"
#include "gfx.h"

static uint32_t surface_width = 0;
static uint32_t surface_height = 0;
static bool opened = false;

bool gfx_open(uint32_t width, uint32_t height) {
    if (!((sys_display_open_t)SYSCALL_TABLE[SYS_DISPLAY_OPEN])(width, height)) return false;

    surface_width = width;
    surface_height = height;
    opened = true;
    return true;
}

bool gfx_close(void) {
    bool was_open = ((sys_display_close_t)SYSCALL_TABLE[SYS_DISPLAY_CLOSE])();

    surface_width = 0;
    surface_height = 0;
    opened = false;
    return was_open;
}

bool gfx_is_open(void) {
    return opened;
}

uint32_t gfx_width(void) {
    return surface_width;
}

uint32_t gfx_height(void) {
    return surface_height;
}

void gfx_present(void) {
    if (!opened) return;
    ((sys_display_present_t)SYSCALL_TABLE[SYS_DISPLAY_PRESENT])();
}

void gfx_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!opened) return;
    ((sys_display_pixel_t)SYSCALL_TABLE[SYS_DISPLAY_PIXEL])(x, y, color);
}

void gfx_fill(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if (!opened) return;
    ((sys_display_fill_t)SYSCALL_TABLE[SYS_DISPLAY_FILL])(x, y, width, height, color);
}

void gfx_clear(uint32_t color) {
    if (!opened) return;
    cursor_erase();
    gfx_fill(0, 0, surface_width, surface_height, color);
    cursor_set(0, 0);
    gfx_present();
}

void gfx_glyph(uint32_t x, uint32_t y, uint8_t ch, uint32_t color) {
    const uint8_t* glyph = font_glyph(ch);
    if (!glyph) return;

    uint32_t scale = font_scale();

    for (uint32_t row = 0; row < 8; row++) {
        uint8_t bits = glyph[row];
        uint32_t col = 0;

        while (col < 8) {
            if (!(bits & (0x80 >> col))) {
                col++;
                continue;
            }

            uint32_t run = 1;
            while (col + run < 8 && (bits & (0x80 >> (col + run)))) run++;

            gfx_fill(x + col * scale, y + row * scale, run * scale, scale, color);
            col += run;
        }
    }
}

void gfx_putc(uint8_t ch, uint32_t color) {
    if (!opened) return;

    uint32_t cell = font_cell();

    cursor_erase();

    if (ch == '\n') {
        cursor_newline();
        return;
    }

    if (ch == '\t') {
        cursor_move(4, 0);
        return;
    }

    if (cursor_col() < CURSOR_MARGIN) {
        cursor_set(CURSOR_MARGIN, cursor_row());
    }

    if ((uint32_t)cursor_pixel_x() + cell > surface_width) {
        cursor_set(CURSOR_MARGIN, cursor_row() + 1);
    }

    if ((uint32_t)cursor_pixel_y() + cell > surface_height) {
        cursor_set(CURSOR_MARGIN, 0);
    }

    gfx_glyph((uint32_t)cursor_pixel_x(), (uint32_t)cursor_pixel_y(), ch, color);
    cursor_move(1, 0);
}
