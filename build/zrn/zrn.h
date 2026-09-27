#ifndef ZRN_H
#define ZRN_H

#include "stdint.h"

#define SYSCALL_TABLE (*((void***)0x50000))

typedef void (*sys_print_t)(const char*, ...);
typedef void (*sys_clear_t)(void);
typedef void (*sys_sleep_t)(uint32_t);

#define print(...) (((sys_print_t)(SYSCALL_TABLE[0]))(__VA_ARGS__))
#define clear()    (((sys_clear_t)(SYSCALL_TABLE[1]))())
#define sleep(ms)  (((sys_sleep_t)(SYSCALL_TABLE[2]))(ms))
#define format_time()    (((sys_clear_t)(SYSCALL_TABLE[3]))())

#endif
