#include "stdint.h"
#include "stddef.h"
#include "map.h"

typedef struct {
    uint8_t map[HEIGHT][WIDTH];
} Level;

static const Level lvl1 = {
    .map = {
        [0] = {0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
        [9] = {0, 0, 0, 0, 0, 2, 0, 0, 0, 0},
    }
};

static const Level lvl2 = {
    .map = {
        [0] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        [1] = {0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
        [9] = {0, 0, 0, 0, 2, 0, 0, 0, 0, 0},
    }
};

static const Level lvl3 = {
    .map = {
        [0] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        [1] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        [2] = {0, 0 ,0, 1, 1, 1, 1, 0, 0, 0},
        [9] = {0, 0, 0, 0, 0, 2, 0, 0, 0, 0},
    }
};

static const Level lvl4 = {
    .map = {
        [0] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        [2] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        [4] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        [5] = {0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
        [6] = {0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
        [9] = {0, 0, 0, 0, 2, 0, 0, 0, 0, 0},
    }
};

const uint8_t (*get_lvl(uint8_t num))[WIDTH] {
    switch (num) {
        case 1:
            return lvl1.map;
            break;
        case 2:
            return lvl2.map;
            break;
        case 3:
            return lvl3.map;
            break;
        case 4:
            return lvl4.map;
            break;
    }

    return NULL;
}
