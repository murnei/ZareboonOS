#ifndef SYSTEM_H
#define SYSTEM_H

#include "stdint.h"

void make_directory(const uint8_t* name);
void remove_directory(const uint8_t* name);
void create_file(const uint8_t* name);
void write_file(const uint8_t* name, const uint8_t* data, uint32_t size);
int32_t read_file(const uint8_t* name, uint8_t* buffer, uint32_t size);

#endif
