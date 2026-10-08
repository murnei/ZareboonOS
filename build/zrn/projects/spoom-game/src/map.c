#include "stdint.h"
#include "stdio.h"
#include "map.h"
#include "player.h"
#include "macro.h"

void draw_frame(uint8_t frame[HEIGHT][WIDTH]) {
    clear();

    for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
            uint8_t num = frame[y][x];

            if (num == 0) {
                print("#");
            } else if (num == 1) {
                print("%C@%C", RED, RESET);
            } else if (num == 2) {
                print( "%C^%C", BLUE, RESET);
            }
		}
        print("\n");
    }
}
