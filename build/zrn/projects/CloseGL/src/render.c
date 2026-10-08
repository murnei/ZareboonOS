#include "stdlib.h"
#include "stdint.h"
#include "math.h"
#include "gfx.h"
#include "render.h"

#define BACKGROUND_COLOR 0x000000
#define FRAME_DELAY 2000000
#define ANGLE_STEP 2

typedef struct {
    float x;
    float y;
    float z;
} vertex_t;

static const vertex_t cube_vertices[8] = {
    {-1.0f, -1.0f, -1.0f},
    { 1.0f, -1.0f, -1.0f},
    { 1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f, -1.0f},
    {-1.0f, -1.0f,  1.0f},
    { 1.0f, -1.0f,  1.0f},
    { 1.0f,  1.0f,  1.0f},
    {-1.0f,  1.0f,  1.0f}
};

static const unsigned char cube_edges[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

static void rect(int x, int y, int width, int height, uint32_t color) {
    if (width <= 0 || height <= 0) return;

    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    if (width <= 0 || height <= 0) return;

    gfx_fill((uint32_t)x, (uint32_t)y, (uint32_t)width, (uint32_t)height, color);
}

void render_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    for (;;) {
        gfx_pixel(x0, y0, color);

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

static vertex_t rotate_vertex(vertex_t v, int ax, int ay, int az) {
    float sax = sin(ax), cax = cos(ax);
    float say = sin(ay), cay = cos(ay);
    float saz = sin(az), caz = cos(az);
    float x, y, z;

    y = v.y * cax - v.z * sax;
    z = v.y * sax + v.z * cax;
    v.y = y;
    v.z = z;

    x = v.x * cay + v.z * say;
    z = -v.x * say + v.z * cay;
    v.x = x;
    v.z = z;

    x = v.x * caz - v.y * saz;
    y = v.x * saz + v.y * caz;
    v.x = x;
    v.y = y;

    return v;
}

void render_cube(int cx, int cy, int size, uint32_t color, int ax, int ay, int az) {
    float half = size / 2.0f;
    float dist = size * 3.0f;
    int px[8];
    int py[8];

    for (int i = 0; i < 8; i++) {
        vertex_t v = cube_vertices[i];
        v.x *= half;
        v.y *= half;
        v.z *= half;

        v = rotate_vertex(v, ax, ay, az);

        float k = dist / (dist + v.z);
        px[i] = cx + (int)(v.x * k);
        py[i] = cy + (int)(v.y * k);
    }

    for (int i = 0; i < 12; i++) {
        int a = cube_edges[i][0];
        int b = cube_edges[i][1];
        render_line(px[a], py[a], px[b], py[b], color);
    }
}

void render_cube_spin(int cx, int cy, int size, uint32_t color) {
    int angle = 0;

    while (gfx_is_open()) {
        gfx_fill(0, 0, gfx_width(), gfx_height(), BACKGROUND_COLOR);
        render_cube(cx, cy, size, color, angle, angle * 2, angle / 2);
        gfx_present();

        angle = (angle + ANGLE_STEP) % 360;

        for (volatile int i = 0; i < FRAME_DELAY; i++) {
        }
    }
}

void render_triangle(int cx, int cy, int size, uint32_t color) {
    for (int i = 0; i < size; i++) {
        gfx_pixel(cx + i, cy + i, color);
        gfx_pixel(cx - i, cy + i, color);
    }

    rect(cx - size, cy + size, 2 * size + 1, 1, color);
}

void render_triangle_fill(int cx, int cy, int size, uint32_t color) {
    for (int y = 1; y < size; y++) {
        rect(cx - y + 1, cy + y, 2 * y - 1, 1, color);
    }
}

void render_square(int cx, int cy, int size, uint32_t color) {
    if (size <= 0) return;

    rect(cx, cy, size, 1, color);
    rect(cx, cy + size - 1, size, 1, color);
    rect(cx, cy, 1, size, color);
    rect(cx + size - 1, cy, 1, size, color);
}

void render_square_fill(int cx, int cy, int size, uint32_t color) {
    rect(cx, cy, size, size, color);
}

static void circle_points(int cx, int cy, int x, int y, uint32_t color) {
    gfx_pixel(cx + x, cy + y, color);
    gfx_pixel(cx - x, cy + y, color);
    gfx_pixel(cx + x, cy - y, color);
    gfx_pixel(cx - x, cy - y, color);

    gfx_pixel(cx + y, cy + x, color);
    gfx_pixel(cx - y, cy + x, color);
    gfx_pixel(cx + y, cy - x, color);
    gfx_pixel(cx - y, cy - x, color);
}

void render_circle(int cx, int cy, int radius, uint32_t color) {
    int x = 0;
    int y = radius;
    int d = 1 - radius;

    while (x <= y) {
        circle_points(cx, cy, x, y, color);

        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }

        x++;
    }
}
