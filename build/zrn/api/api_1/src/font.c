#include "stdint.h"
#include "zrn_syscall.h"
#include "font.h"

static uint8_t scale = 2;

uint8_t font_scale(void) {
    return scale;
}

void font_set_scale(uint8_t value) {
    if (value == 0) return;
    scale = value;
}

uint32_t font_cell(void) {
    return FONT_SIZE * scale;
}

const uint8_t* font_glyph(uint8_t ch) {
    return ((sys_get_font_t)SYSCALL_TABLE[SYS_GET_FONT])(ch);
}
