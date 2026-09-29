#include "stdio.h"
#include "shell.h"

void kernel_main() {
    shell_main();

    __asm__ volatile(
        "hlt"
    );

}
