#ifndef STDLIB_H
#define STDLIB_H

#include "stdint.h"
#include "stdbool.h"

typedef struct block_header {
    uint32_t size;
    struct block_header* next;
} block_header_t;

void heap_init();
void* kmalloc(uint32_t size);
void kfree(void* ptr);

#endif
