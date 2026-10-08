#include "zrn_syscall.h"

char getch(void) {
    return ((sys_getch_t)SYSCALL_TABLE[6])();
}
