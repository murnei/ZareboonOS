#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "video.h"
#include "display.h"

#define DISPLAY_MAX_PIXELS (1u << 24)

typedef struct {
    uint32_t* back;
    uint32_t* front;
    uint32_t width;
    uint32_t height;
    uint32_t dirty_top;
    uint32_t dirty_bottom;
    bool full;
    bool active;
} display_state_t;

static display_state_t display;

static void mark_dirty(uint32_t top, uint32_t bottom) {
    if (display.dirty_top >= display.dirty_bottom) {
        display.dirty_top = top;
        display.dirty_bottom = bottom;
        return;
    }
    if (top < display.dirty_top) display.dirty_top = top;
    if (bottom > display.dirty_bottom) display.dirty_bottom = bottom;
}

static void clear_dirty(void) {
    display.dirty_top = 0;
    display.dirty_bottom = 0;
}

static void free_buffers(void) {
    if (display.back) kfree(display.back);
    if (display.front) kfree(display.front);
    display.back = 0;
    display.front = 0;
}

bool display_open(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) return false;
    if (width > DISPLAY_MAX_PIXELS / height) return false;

    uint32_t pixels = width * height;

    uint32_t* back = (uint32_t*)kmalloc(pixels * sizeof(uint32_t));
    if (!back) return false;

    uint32_t* front = (uint32_t*)kmalloc(pixels * sizeof(uint32_t));
    if (!front) {
        kfree(back);
        return false;
    }

    for (uint32_t i = 0; i < pixels; i++) {
        back[i] = 0;
        front[i] = 0;
    }

    bool was_active = display.active;

    free_buffers();

    display.back = back;
    display.front = front;
    display.width = width;
    display.height = height;
    display.full = true;
    display.active = true;
    clear_dirty();
    mark_dirty(0, height);

    if (!was_active) console_suspend();
    return true;
}

bool display_close(void) {
    bool was_active = display.active;

    free_buffers();

    display.width = 0;
    display.height = 0;
    display.full = false;
    display.active = false;
    clear_dirty();

    if (was_active) console_resume();
    return was_active;
}

bool display_active(void) {
    return display.active;
}

display_mode_t display_mode(void) {
    return display.active ? DISPLAY_GRAPHICS : DISPLAY_TEXT;
}

void display_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!display.active || x >= display.width || y >= display.height) return;

    uint32_t* pixel = &display.back[y * display.width + x];
    if (*pixel == color) return;

    *pixel = color;
    mark_dirty(y, y + 1);
}

void display_fill(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if (!display.active || x >= display.width || y >= display.height) return;
    if (width > display.width - x) width = display.width - x;
    if (height > display.height - y) height = display.height - y;
    if (width == 0 || height == 0) return;

    for (uint32_t row = y; row < y + height; row++) {
        uint32_t* dst = &display.back[row * display.width + x];
        uint32_t count = width;
        __asm__ volatile("cld; rep stosl" : "+D"(dst), "+c"(count) : "a"(color) : "memory");
    }
    mark_dirty(y, y + height);
}

void display_present(void) {
    if (!display.active) return;

    bool full = display.full;
    uint32_t top = display.dirty_top;
    uint32_t bottom = display.dirty_bottom;

    if (full) {
        top = 0;
        bottom = display.height;
        display.full = false;
    }

    if (top >= bottom) return;

    for (uint32_t y = top; y < bottom; y++) {
        uint32_t* back = &display.back[y * display.width];
        uint32_t* front = &display.front[y * display.width];

        if (!full) {
            bool changed = false;
            for (uint32_t x = 0; x < display.width; x++) {
                if (back[x] != front[x]) {
                    changed = true;
                    break;
                }
            }
            if (!changed) continue;
        }

        blit32(display.back, display.width, y, y + 1);

        for (uint32_t x = 0; x < display.width; x++) front[x] = back[x];
    }

    clear_dirty();
}
