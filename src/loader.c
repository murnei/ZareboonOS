#include "stdint.h"
#include "stdbool.h"
#include "fs.h"
#include "stdio.h"
#include "display.h"

#define SYSCALL_TABLE_PTR_ADDR 0X50000
#define PROGRAM_LOAD_ADDR 0x60000
#define PROGRAM_MAX_SIZE  0x3F000

extern void* get_syscall_table(void);

void clear(void);

volatile bool app_running = false;
static void* app_jmp[5];

void app_abort(void) {
    if (!app_running) return;
    app_running = false;
    __builtin_longjmp(app_jmp, 1);
}

static void run_program(uint32_t entry_addr, void* syscall_table_ptr) {
    void* eax_val = syscall_table_ptr;

    __asm__ volatile (
        "call *%1\n"
        : "+a"(eax_val)
        : "r"(entry_addr)
        : "ecx", "edx", "cc", "memory"
    );
}

bool exec_program(const uint8_t* filename) {
    file_info_t info;

    if (!fs_find_file(filename, &info)) {
        print("File not found");
        return false;
    }

    if (info.file_size == 0 || info.file_size > PROGRAM_MAX_SIZE) {
        print("File too large or empty");
        return false;
    }

    uint8_t* load_addr = (uint8_t*)PROGRAM_LOAD_ADDR;
    uint32_t read = fs_read_file(&info, load_addr, PROGRAM_MAX_SIZE);

    if (read != info.file_size) {
        print("Failed to read full file");
        return false;
    }

    *(void**)SYSCALL_TABLE_PTR_ADDR = get_syscall_table();

    app_running = true;

    if (__builtin_setjmp(app_jmp) == 0) {
        run_program(PROGRAM_LOAD_ADDR, get_syscall_table());
        app_running = false;
        return true;
    }

    __asm__ volatile ("sti");
    if (display_close()) {
        clear();
    }
    print("^C");
    return false;
}
