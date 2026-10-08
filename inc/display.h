#ifndef DISPLAY_H
#define DISPLAY_H

#include "stdint.h"
#include "stdbool.h"

typedef enum {
    DISPLAY_TEXT,
    DISPLAY_GRAPHICS
} display_mode_t;

bool display_open(uint32_t width, uint32_t height);
bool display_close(void);
bool display_active(void);
display_mode_t display_mode(void);

void display_pixel(uint32_t x, uint32_t y, uint32_t color);
void display_fill(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void display_present(void);

void console_suspend(void);
void console_resume(void);

#endif
