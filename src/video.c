#include "font.h"
#include "stdint.h"

static volatile uint8_t* vbe_buffer = (volatile unsigned char*)0xE0000000;

uint16_t vbe_width = 1024;
uint16_t vbe_height = 768;
uint16_t vbe_pitch = 4096;
uint16_t vbe_bpp = 32;

typedef struct {
    unsigned int framebuffer;
    unsigned short pitch;
    unsigned short width;
    unsigned short height;
    unsigned char bpp;
} GraphicInfo;

void graphic_init(GraphicInfo* info) {
    if (info != 0) {
        vbe_buffer = (volatile unsigned char*)info->framebuffer;
        vbe_width = info->width;
        vbe_height = info->height;
        vbe_pitch = info->pitch;
        vbe_bpp = info->bpp;
    }
}

void put_pixel(int x, int y, unsigned int color) {
    if (x >= vbe_width || x < 0 || y < 0 || y >= vbe_height) {
        return;
    }

    unsigned char* row = (unsigned char*)vbe_buffer + y * vbe_pitch;

    if (vbe_bpp == 32) {
        unsigned int* pixel = (unsigned int*)(row + x * 4);
        *pixel = color;
    } else if (vbe_bpp == 24) {
        unsigned char* pixel = row + x * 3;
        pixel[0] = (unsigned char)(color & 0xFF);
        pixel[1] = (unsigned char)((color >> 8) & 0xFF);
        pixel[2] = (unsigned char)((color >> 16) & 0xFF);
    }
}

static inline void fill32(volatile uint32_t* dst, uint32_t color, uint32_t count) {
    __asm__ volatile(
        "rep stosl"
        : "+D"(dst), "+c"(count)
        : "a"(color)
        : "memory"
    );
}

void video_memset(uint16_t x, uint16_t y, uint32_t color, uint32_t count) {
    if (x >= vbe_width || y >= vbe_height) return;

    uint32_t remaining = count;
    uint16_t cur_x = x;
    uint16_t cur_y = y;

    if (vbe_bpp == 32) {
        while (remaining > 0 && cur_y < vbe_height) {
            uint32_t row_space = vbe_width - cur_x;
            uint32_t chunk = (remaining < row_space) ? remaining : row_space;

            volatile uint32_t* dest = (volatile uint32_t*)(vbe_buffer
                + (uint32_t)cur_y * vbe_pitch
                + (uint32_t)cur_x * 4);

            fill32(dest, color, chunk);

            remaining -= chunk;
            cur_x = 0;
            cur_y++;
        }
    } else if (vbe_bpp == 24) {
        uint8_t b0 = (uint8_t)(color & 0xFF);
        uint8_t b1 = (uint8_t)((color >> 8) & 0xFF);
        uint8_t b2 = (uint8_t)((color >> 16) & 0xFF);

        while (remaining > 0 && cur_y < vbe_height) {
            uint32_t row_space = vbe_width - cur_x;
            uint32_t chunk = (remaining < row_space) ? remaining : row_space;

            volatile uint8_t* dest = vbe_buffer
                + (uint32_t)cur_y * vbe_pitch
                + (uint32_t)cur_x * 3;

            for (uint32_t i = 0; i < chunk; i++) {
                dest[0] = b0;
                dest[1] = b1;
                dest[2] = b2;
                dest += 3;
            }

            remaining -= chunk;
            cur_x = 0;
            cur_y++;
        }
    }
}

void fill_rect(int x, int y, int w, int h, uint32_t color) {
    if (x < 0 || y < 0) return;
    if (x >= vbe_width || y >= vbe_height) return;
    if (x + w > vbe_width)  w = vbe_width  - x;
    if (y + h > vbe_height) h = vbe_height - y;
    if (w <= 0 || h <= 0) return;

    if (vbe_bpp == 32) {
        for (int py = 0; py < h; py++) {
            volatile uint32_t* dst = (volatile uint32_t*)(vbe_buffer
                + (uint32_t)(y + py) * vbe_pitch
                + (uint32_t)x * 4);
            fill32(dst, color, (uint32_t)w);
        }
    } else {
        for (int py = 0; py < h; py++)
            for (int px = 0; px < w; px++)
                put_pixel(x + px, y + py, color);
    }
}

void blit32(const uint32_t* src, uint32_t src_w, uint32_t y0, uint32_t y1) {
    uint32_t w = src_w < vbe_width ? src_w : vbe_width;
    if (y1 > vbe_height) y1 = vbe_height;

    if (vbe_bpp != 32) {
        for (uint32_t y = y0; y < y1; y++)
            for (uint32_t x = 0; x < w; x++)
                put_pixel(x, y, src[y * src_w + x]);
        return;
    }

    for (uint32_t y = y0; y < y1; y++) {
        volatile uint32_t* dst = (volatile uint32_t*)(vbe_buffer + y * vbe_pitch);
        const uint32_t* s = src + y * src_w;
        uint32_t cnt = w;
        __asm__ volatile("cld; rep movsl" : "+D"(dst), "+S"(s), "+c"(cnt) : : "memory");
    }
}
