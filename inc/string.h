#ifndef STRING_H
#define STRING_H

#include "stdint.h"

typedef struct {
    char* data;
    uint32_t length;
    uint32_t capacity;
} string;

string* str_create(uint32_t initial_capacity) {
    string* s = (string*)kmalloc(sizeof(string));
    if (!s) return NULL;

    s->data = (char*)kmalloc(initial_capacity);
    if (!s->data) {
        kfree(s);
        return NULL;
    }

    s->length = 0;
    s->capacity = initial_capacity;
    s->data[0] = '\0';
    return s;
}

bool strcmp(const uint8_t* ch1, const uint8_t* ch2);

#endif
