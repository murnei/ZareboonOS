#ifndef FS_H
#define FS_H

#include "stdint.h"
#include "stdbool.h"

typedef struct {
    uint32_t start_cluster;
    uint32_t file_size;
} file_info_t;

void fs_init();
bool fs_find_file(const uint8_t* filename, file_info_t* out);
uint32_t fs_read_file(const file_info_t* info, uint8_t* dest, uint32_t max_size);
uint32_t fs_cluster_size(void);
void fat32_list_files(void);

#endif
