#ifndef ATA_H
#define ATA_H

#include "stdint.h"
#include "stdbool.h"

void ata_init(void);

extern uint8_t ata_last_status;
extern uint8_t ata_last_error;
extern int ata_found_index;

bool ata_read_sector(uint32_t lba, void* buffer);
bool ata_write_sector(uint32_t lba, const void* buffer);

#endif
