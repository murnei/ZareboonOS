#ifndef VIDEO_H
#define VIDEO_H

#include "stdint.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

typedef struct {
    unsigned int framebuffer;
    unsigned short pitch;
    unsigned short width;
    unsigned short height;
    unsigned char bpp;
} GraphicInfo;

void graphic_init(GraphicInfo* info);
void put_pixel(int x, int y, unsigned int color);
void video_memset(uint16_t x, uint16_t y, uint32_t color, uint32_t count);

static inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (255U << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

#endif
