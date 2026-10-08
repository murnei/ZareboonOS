#include "stdint.h"
#include "time.h"
#include "display.h"
#include "fs.h"
#include "args.h"

void clear(void);
void print_char(uint8_t ch, uint32_t color);
void sleep(const uint32_t ms);
void get_time(struct time_now* t);
void make_dir(uint8_t* name);
void remove_dir(uint8_t* name);
int fs_create_file(const uint8_t* name);
int fs_write_file(const uint8_t* name, const uint8_t* data, uint32_t size);
char keyboard_read_char(void);
const uint8_t* get_font(uint8_t c);

typedef enum {
    SYS_CLEAR = 0,
    SYS_SLEEP = 1,
    SYS_GET_TIME = 2,
    SYS_DISPLAY_OPEN = 3,
    SYS_DISPLAY_PIXEL = 4,
    SYS_DISPLAY_PRESENT = 5,
    SYS_GETCH = 6,
    SYS_GET_FONT = 7,
    SYS_DISPLAY_FILL = 8,
    SYS_DISPLAY_CLOSE = 9,
    SYS_MKDIR = 10,
    SYS_RMDIR = 11,
    SYS_PUTC = 12,
    SYS_CREATE_FILE = 13,
    SYS_WRITE_FILE = 14,
    SYS_READ_FILE = 15,
    SYS_ARGS = 16,
    SYS_COUNT
} syscall_id_t;

static void* syscall_table[SYS_COUNT];

static int syscall_unsupported(void) {
    return -1;
}

void syscalls_init(void) {
    for (uint32_t i = 0; i < SYS_COUNT; i++) {
        syscall_table[i] = (void*)syscall_unsupported;
    }

    syscall_table[SYS_CLEAR] = (void*)clear;
    syscall_table[SYS_SLEEP] = (void*)sleep;
    syscall_table[SYS_GET_TIME] = (void*)get_time;
    syscall_table[SYS_GETCH] = (void*)keyboard_read_char;
    syscall_table[SYS_GET_FONT] = (void*)get_font;
    syscall_table[SYS_DISPLAY_OPEN] = (void*)display_open;
    syscall_table[SYS_DISPLAY_PIXEL] = (void*)display_pixel;
    syscall_table[SYS_DISPLAY_PRESENT] = (void*)display_present;
    syscall_table[SYS_DISPLAY_FILL] = (void*)display_fill;
    syscall_table[SYS_DISPLAY_CLOSE] = (void*)display_close;
    syscall_table[SYS_MKDIR] = (void*)make_dir;
    syscall_table[SYS_RMDIR] = (void*)remove_dir;
    syscall_table[SYS_PUTC] = (void*)print_char;
    syscall_table[SYS_CREATE_FILE] = (void*)fs_create_file;
    syscall_table[SYS_WRITE_FILE] = (void*)fs_write_file;
    syscall_table[SYS_ARGS] = (void*)args_get;
}

void* get_syscall_table(void) {
    return (void*)syscall_table;
}
