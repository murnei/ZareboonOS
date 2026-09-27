#include "stdint.h"
#include "stdbool.h"

bool strcmp(const uint8_t* ch1, const uint8_t* ch2) {
    uint16_t i = 0;

    while (1) {
        if (ch1[i] != ch2[i]) return false;

        if (ch1[i] == '\0') return true;

        i++;
    }
}
