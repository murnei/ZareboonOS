#ifndef FS_H
#define FS_H

#include "stdint.h"
#include "stdbool.h"

typedef struct {
    uint32_t start_cluster;
    uint32_t file_size;
} file_info_t;

#define FS_PARTITION_LBA 64

#ifndef FS_OK
#define FS_OK            0
#define FS_ERR_NOFS     -1
#define FS_ERR_EXISTS   -2
#define FS_ERR_NOTFOUND -3
#define FS_ERR_NOTDIR   -4
#define FS_ERR_NOTEMPTY -5
#define FS_ERR_NOSPACE  -6
#define FS_ERR_IO       -7
#define FS_ERR_ISDIR    -8
#define FS_ERR_DIRFULL  -9
#endif

void fs_init();
bool fs_find_file(const uint8_t* filename, file_info_t* out);
uint32_t fs_read_file(const file_info_t* info, uint8_t* dest, uint32_t max_size);
uint32_t fs_cluster_size(void);
void fat32_list_files(void);
void make_dir(uint8_t* name);
void remove_dir(uint8_t* name);
int fs_cat(const uint8_t* name);
int fs_create_file(const uint8_t* name);
int fs_write_file(const uint8_t* name, const uint8_t* data, uint32_t size);

#endif
