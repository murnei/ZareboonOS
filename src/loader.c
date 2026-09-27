#include "fs.h"

#include "stdio.h"



#define SYSCALL_TABLE_PTR_ADDR 0X50000

#define PROGRAM_LOAD_ADDR 0x60000

#define PROGRAM_MAX_SIZE  0x3F000



extern void* get_syscall_table(void);



static void run_program(uint32_t entry_addr, void* syscall_table_ptr) {

    __asm__ volatile (

        "call *%1\n"

        :

        : "a"(syscall_table_ptr), "r"(entry_addr)

        : "memory"

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

    run_program(PROGRAM_LOAD_ADDR, get_syscall_table());

    return true;

}
