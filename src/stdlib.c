#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"

#define RAM_HEAP 0x01000000
#define HEAP_SIZE 0x00100000

static block_header_t* free_list = NULL;

void heap_init(void) {
    block_header_t* first_block = (block_header_t*)RAM_HEAP;

    first_block->size = HEAP_SIZE - sizeof(block_header_t);
    first_block->size &= ~1;
    first_block->next = NULL;
    free_list = first_block;
}

void* kmalloc(uint32_t size) {
    __asm__ volatile ("cli");

    if (size == 0) {
        __asm__ volatile ("sti");
        return NULL;
    }

    uint32_t aligned_size = (size + 3) & ~3;
    block_header_t* curr = free_list;

    while (curr != NULL) {
        if ((curr->size & 1) == 0 && (curr->size & ~1) >= aligned_size) {
            uint32_t real_size = curr->size & ~1;

            if (real_size >= aligned_size + sizeof(block_header_t) + 4) {
                uint32_t new_block_addr = (uint32_t)curr + sizeof(block_header_t) + aligned_size;
                block_header_t* new_block = (block_header_t*)new_block_addr;

                new_block->size = real_size - aligned_size - sizeof(block_header_t);
                new_block->size &= ~1;
                new_block->next = curr->next;

                curr->size = aligned_size;
                curr->next = new_block;
            } else {
                curr->size = real_size;
            }

            curr->size |= 1;

            __asm__ volatile ("sti");
            return (void*)(curr + 1);
        }

        curr = curr->next;
    }

    __asm__ volatile ("sti");
    return NULL;
}

void kfree(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    __asm__ volatile ("cli");

    block_header_t* target = (block_header_t*)ptr - 1;
    target->size &= ~1;

    block_header_t* curr = free_list;
    while (curr != NULL && curr->next != NULL) {
        uint32_t curr_end = (uint32_t)curr + sizeof(block_header_t) + (curr->size & ~1);

        if (curr_end == (uint32_t)curr->next) {
            curr->size = (curr->size & ~1) + sizeof(block_header_t) + (curr->next->size & ~1);
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }

    __asm__ volatile ("sti");
}
