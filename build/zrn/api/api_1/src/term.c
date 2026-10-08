#include "stdint.h"
#include "zrn_syscall.h"
#include "gfx.h"
#include "term.h"

void term_putc(char ch, uint32_t color) {
    if (gfx_is_open()) return;
    ((sys_putc_t)SYSCALL_TABLE[SYS_PUTC])((uint8_t)ch, color);
}

void term_write(const char* text, uint32_t color) {
    if (!text) return;
    while (*text) term_putc(*text++, color);
}

void term_clear(void) {
    if (gfx_is_open()) return;
    ((sys_clear_t)SYSCALL_TABLE[SYS_CLEAR])();
}
