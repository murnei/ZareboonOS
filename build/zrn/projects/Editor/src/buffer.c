#include "stdint.h"
#include "stdbool.h"
#include "buffer.h"

static uint32_t line_start(const buffer_t* buffer) {
    uint32_t index = buffer->length;
    while (index > 0 && buffer->data[index - 1] != '\n') index--;
    return index;
}

void buffer_clear(buffer_t* buffer) {
    buffer->length = 0;
}

bool buffer_push(buffer_t* buffer, uint8_t ch) {
    if (buffer->length >= BUFFER_CAPACITY) return false;
    buffer->data[buffer->length++] = ch;
    return true;
}

bool buffer_pop(buffer_t* buffer) {
    if (buffer->length == 0) return false;
    buffer->length--;
    return true;
}

bool buffer_line_is(const buffer_t* buffer, const char* text) {
    uint32_t start = line_start(buffer);
    uint32_t index = 0;

    while (text[index]) {
        if (start + index >= buffer->length) return false;
        if (buffer->data[start + index] != (uint8_t)text[index]) return false;
        index++;
    }

    return start + index == buffer->length;
}

void buffer_drop_line(buffer_t* buffer) {
    buffer->length = line_start(buffer);
}
