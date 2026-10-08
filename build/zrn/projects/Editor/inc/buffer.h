#ifndef BUFFER_H
#define BUFFER_H

#include "stdint.h"
#include "stdbool.h"

#define BUFFER_CAPACITY 4096

typedef struct {
    uint8_t data[BUFFER_CAPACITY];
    uint32_t length;
} buffer_t;

void buffer_clear(buffer_t* buffer);
bool buffer_push(buffer_t* buffer, uint8_t ch);
bool buffer_pop(buffer_t* buffer);
bool buffer_line_is(const buffer_t* buffer, const char* text);
void buffer_drop_line(buffer_t* buffer);

#endif
