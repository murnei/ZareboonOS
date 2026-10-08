#include "zrn.h"
#include "stdbool.h"
#include "stdio.h"
#include "screen.h"
#include "video.h"
#include "cursor.h"
#include "time.h"

ZRN_ENTRY
void main() {
	screen_init(1024, 768);
	set_cursor_visible(false);

    while (true) {
        clear();
        print_format_time();
        sleep(1000);
    }

    return;
}
