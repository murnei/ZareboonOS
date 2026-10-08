#ifndef PLAYER_H
#define PLAYER_H

#include "stdint.h"

struct Player {
    uint8_t x;
    uint8_t y;
};

extern struct Player player;

#endif
