#ifndef VIDEO_H
#define VIDEO_H

static inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (255U << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

#endif
