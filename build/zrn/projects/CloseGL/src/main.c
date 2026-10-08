#include "zrn.h"
#include "stdbool.h"
#include "stdio.h"
#include "gfx.h"
#include "cursor.h"
#include "render.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

static bool project_init(void) {
    if (!gfx_open(SCREEN_WIDTH, SCREEN_HEIGHT)) return false;
    set_cursor_visible(false);
    return true;
}

ZRN_ENTRY
void main(void) {
    if (!project_init()) return;

    render_cube_spin(gfx_width() / 2, gfx_height() / 2, 150, 0x00FFFFFF);

    gfx_close();
}
