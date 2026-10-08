#include "zrn.h"
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "args.h"
#include "gfx.h"
#include "system.h"

static bool wants_gfx(void) {
    return args_count() > 0 && args_at(0)[0] == 'g';
}

ZRN_ENTRY
void main(void) {
    print("1 start\n");

    uint32_t count = args_count();
    print("2 args=%u\n", count);

    uint8_t data[16];
    print("3 read=%d\n", read_file((const uint8_t*)"note.txt", data, 8));

    if (!wants_gfx()) {
        print("done");
        return;
    }

    bool opened = gfx_open(1024, 768);
    gfx_fill(10, 10, 100, 100, gfx_rgb(255, 0, 0));
    gfx_present();
    gfx_close();

    print("4 gfx=%d", opened);
}
