#ifndef STRING_H
#define STRING_H

#include "stdint.h"
#include "stddef.h"

void memcpy(void* dest, const void* src, size_t n);
uint16_t strlen(const char* c);

#endif
