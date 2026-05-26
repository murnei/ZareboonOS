#ifndef DISK_H
#define DISK_H

void read_sector(uint32_t lba, uint8_t* buffer);
void write_sector(uint32_t lba, uint8_t* buffer);

#endif
