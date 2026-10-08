#include "zrn.h"
#include "game.h"
#include "screen.h"
#include "cursor.h"

ZRN_ENTRY
void main() {
    screen_init(1024, 768);
    set_cursor_visible(false);
    update();

    return;
}
