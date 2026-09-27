#include "stdio.h"
#include "shell.h"

void kernel_main() {
    shell_main();

    // draw_char(get_raw_cursor_x(), get_raw_cursor_y(), 150, 0x00FF0000, 50);

    __asm__ volatile(
        "hlt"
    );

}
