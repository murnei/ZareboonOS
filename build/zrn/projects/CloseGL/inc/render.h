#ifndef RENDER_H
#define RENDER_H

#include "stdint.h"

void render_line(int x0, int y0, int x1, int y1, uint32_t color);
void render_cube(int cx, int cy, int size, uint32_t color, int ax, int ay, int az);
void render_cube_spin(int cx, int cy, int size, uint32_t color);
void render_triangle(int cx, int cy, int size, uint32_t color);
void render_triangle_fill(int cx, int cy, int size, uint32_t color);
void render_square(int cx, int cy, int size, uint32_t color);
void render_square_fill(int cx, int cy, int size, uint32_t color);
void render_circle(int cx, int cy, int radius, uint32_t color);

#endif
