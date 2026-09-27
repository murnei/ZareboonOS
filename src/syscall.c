#include "stdint.h"

void print(const char* s);
void clear(void);
void sleep(const uint32_t ms);
void format_time();

typedef enum {
    SYS_PRINT = 0,
    SYS_CLEAR = 1,
    SYS_SLEEP = 2,
    SYS_FORMAT_TIME = 3,
    SYS_COUNT
} syscall_id_t;

static void* syscall_table[SYS_COUNT];

void syscalls_init(void) {
    syscall_table[SYS_PRINT] = (void*)print;
    syscall_table[SYS_CLEAR] = (void*)clear;
    syscall_table[SYS_SLEEP] = (void*)sleep;
    syscall_table[SYS_FORMAT_TIME] = (void*)format_time;
}

void* get_syscall_table(void) {
    return (void*)syscall_table;
}
