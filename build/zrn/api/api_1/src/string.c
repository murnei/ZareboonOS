#include "stdint.h"
#include "stddef.h"

void memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = dest;
    const uint8_t* s = src;

    while (n--) {
        *d++ =  *s++;
    }
}

uint16_t strlen(const char* c) {
    int i = 0;

    while(c[i] != '\0') {
        i++;
    }

    return i;
}
