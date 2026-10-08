#ifndef ARGS_H
#define ARGS_H

#include "stdint.h"

uint32_t args_count(void);
const char* args_at(uint32_t index);
const char* args_line(void);

#endif
