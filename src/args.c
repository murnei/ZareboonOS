#include "stdint.h"
#include "args.h"

#define ARGS_CAPACITY 256

static uint8_t line[ARGS_CAPACITY];

void args_set(const uint8_t* text) {
    uint32_t length = 0;

    if (text) {
        while (text[length] && length < ARGS_CAPACITY - 1) {
            line[length] = text[length];
            length++;
        }
    }

    line[length] = '\0';
}

const uint8_t* args_get(void) {
    return line;
}
