#include "stdint.h"
#include "zrn_syscall.h"
#include "system.h"

void make_directory(const uint8_t* name) {
    ((sys_mkdir_t)SYSCALL_TABLE[SYS_MKDIR])((uint8_t*)name);
}

void remove_directory(const uint8_t* name) {
    ((sys_rmdir_t)SYSCALL_TABLE[SYS_RMDIR])((uint8_t*)name);
}

void create_file(const uint8_t* name) {
    ((sys_create_file_t)SYSCALL_TABLE[SYS_CREATE_FILE])(name);
}

void write_file(const uint8_t* name, const uint8_t* data, uint32_t size) {
    ((sys_write_file_t)SYSCALL_TABLE[SYS_WRITE_FILE])(name, data, size);
}

int32_t read_file(const uint8_t* name, uint8_t* buffer, uint32_t size) {
    return ((sys_read_file_t)SYSCALL_TABLE[SYS_READ_FILE])(name, buffer, size);
}
