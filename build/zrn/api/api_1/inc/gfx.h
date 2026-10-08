#ifndef GFX_H
#define GFX_H

#include "stdint.h"
#include "stdbool.h"

static inline uint32_t gfx_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

bool gfx_open(uint32_t width, uint32_t height);
bool gfx_close(void);
bool gfx_is_open(void);
uint32_t gfx_width(void);
uint32_t gfx_height(void);

void gfx_present(void);
void gfx_pixel(uint32_t x, uint32_t y, uint32_t color);
void gfx_fill(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void gfx_clear(uint32_t color);

void gfx_glyph(uint32_t x, uint32_t y, uint8_t ch, uint32_t color);
void gfx_putc(uint8_t ch, uint32_t color);

#endif
