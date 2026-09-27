#ifndef FLOPPY_H
#define FLOPPY_H

#include "stdint.h"
#include "stdbool.h"

bool floppy_init(void);
bool floppy_read_sector(uint32_t lba, uint8_t* buffer);

#endif
