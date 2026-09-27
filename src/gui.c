#include "video.h"
#include "stdint.h"

void draw_rect(uint16_t length, uint16_t start_x, uint16_t start_y, uint32_t color) {
    for (uint16_t x = 0; x < length; x++) {
        put_pixel(start_x + x, start_y, color);
    }
}

void draw_square(uint16_t square) {
    for (uint16_t i = 0; i < square; i++) {

    }
}
