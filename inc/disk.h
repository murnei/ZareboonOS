#pragma once
#include "stdint.h"
#include "stdbool.h"

typedef struct {
    const char* name;
    bool (*read)(uint32_t lba, uint8_t* buffer);
} disk_driver_t;

void disk_register(const disk_driver_t* driver);
bool disk_read_sector(uint32_t lba, uint8_t* buffer);
const char* disk_name(void);
