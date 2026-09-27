#include "stdint.h"
#include "stdbool.h"
#include "io.h"
#include "console.h"
#include "stdio.h"
#include "video.h"
#include "fs.h"
#include "floppy.h"

#define FS_PARTITION_LBA 34

static uint16_t bytes_per_sec = 0;
static uint8_t  sec_per_clus = 0;
static uint16_t reserved_sectors = 0;
static uint8_t  num_fats = 0;
static uint32_t fat_size_32 = 0;
static uint32_t root_cluster = 0;
static uint32_t data_sec = 0;
static uint32_t partition_lba = 0;
static bool fs_ready = false;

static uint8_t sector_buffer[512];

typedef struct {
    uint8_t     name[11];
    uint8_t     attributes;
    uint8_t     reserved;
    uint8_t     created_time_ms;
    uint16_t    created_time;
    uint16_t    created_date;
    uint16_t    accessed_date;
    uint16_t    cluster_high;
    uint16_t    modified_time;
    uint16_t    modified_date;
    uint16_t    cluster_low;
    uint32_t    file_size;
} __attribute__((packed)) fat_entry_t;

void fs_init(void) {
    partition_lba = FS_PARTITION_LBA;

    floppy_read_sector(partition_lba, sector_buffer);  // <-- было 0

    bytes_per_sec    = *(uint16_t*)(sector_buffer + 0x0B);
    sec_per_clus     = *(uint8_t*)(sector_buffer + 0x0D);
    reserved_sectors = *(uint16_t*)(sector_buffer + 0x0E);
    num_fats         = *(uint8_t*)(sector_buffer + 0x10);
    fat_size_32      = *(uint32_t*)(sector_buffer + 0x24);
    root_cluster     = *(uint32_t*)(sector_buffer + 0x2C);

    if (bytes_per_sec != 512 || sec_per_clus == 0) {
        print("FS_INIT_FAILED");
        fs_ready = false;
        return;
    }

    data_sec = reserved_sectors + (num_fats * fat_size_32);

    fs_ready = true;
}

uint32_t fs_cluster_size(void) {
    return (uint32_t)bytes_per_sec * (uint32_t)sec_per_clus;
}

static uint32_t calculate_LBA(uint32_t cluster) {
    return partition_lba + data_sec + (cluster - 2) * sec_per_clus;  // <-- добавлен partition_lba
}

static void read_cluster(uint32_t cluster, uint8_t* buffer) {
    if (!fs_ready) return;

    uint32_t start_lba = calculate_LBA(cluster);
    for (uint8_t i = 0; i < sec_per_clus; i++) {
        floppy_read_sector(start_lba + i, buffer + (i * 512));
    }
}

static uint32_t get_next_cluster(uint32_t cluster) {
    if (!fs_ready || bytes_per_sec == 0) {
        return 0x0FFFFFFF;
    }

    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = partition_lba + reserved_sectors + (fat_offset / bytes_per_sec);  // <-- добавлен partition_lba
    uint32_t ent_offset = fat_offset % bytes_per_sec;

    static uint8_t fat_buffer[512];
    floppy_read_sector(fat_sector, fat_buffer);

    uint32_t next_cluster = *(uint32_t*)&fat_buffer[ent_offset];
    return next_cluster & 0x0FFFFFFF;
}

static void format_fat_name(const uint8_t* user_name, uint8_t out[11]) {
    for (int i = 0; i < 11; i++) out[i] = ' ';

    int i = 0, j = 0;
    while (user_name[i] != '\0' && user_name[i] != '.' && j < 8) {
        uint8_t c = user_name[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        out[j++] = c;
        i++;
    }
    while (user_name[i] != '\0' && user_name[i] != '.') i++;

    if (user_name[i] == '.') {
        i++;
        int k = 8;
        while (user_name[i] != '\0' && k < 11) {
            uint8_t c = user_name[i];
            if (c >= 'a' && c <= 'z') c -= 32;
            out[k++] = c;
            i++;
        }
    }
}

static bool fat_name_equal(const uint8_t* raw11, const uint8_t formatted[11]) {
    for (int n = 0; n < 11; n++) {
        if (raw11[n] != formatted[n]) return false;
    }
    return true;
}

bool fs_find_file(const uint8_t* filename, file_info_t* out) {
    if (!fs_ready) {
        print("Filesystem not available");
        return false;
    }

    uint8_t target[11];
    format_fat_name(filename, target);

    static uint8_t root_buffer[16384];
    uint32_t cur_clus = root_cluster;

    while (cur_clus >= 2 && cur_clus < 0x0FFFFFF8) {
        read_cluster(cur_clus, root_buffer);
        fat_entry_t* entries = (fat_entry_t*)root_buffer;

        uint32_t entries_per_cluster = fs_cluster_size() / 32;
        if (entries_per_cluster == 0) break;

        for (uint32_t i = 0; i < entries_per_cluster; i++) {
            if (entries[i].name[0] == 0x00) return false;
            if (entries[i].name[0] == 0xE5) continue;
            if (entries[i].attributes == 0x0F) continue;
            if (entries[i].attributes & 0x08) continue;
            if (entries[i].attributes & 0x10) continue;

            if (fat_name_equal(entries[i].name, target)) {
                out->start_cluster = ((uint32_t)entries[i].cluster_high << 16) | entries[i].cluster_low;
                out->file_size = entries[i].file_size;
                return true;
            }
        }

        cur_clus = get_next_cluster(cur_clus);
    }

    return false;
}

uint32_t fs_read_file(const file_info_t* info, uint8_t* dest, uint32_t max_size) {
    if (!fs_ready) return 0;

    uint32_t cluster = info->start_cluster;
    uint32_t bytes_left = (info->file_size < max_size) ? info->file_size : max_size;
    uint32_t total_read = 0;
    uint32_t cluster_bytes = fs_cluster_size();

    static uint8_t cluster_buf[16384];

    while (cluster >= 2 && cluster < 0x0FFFFFF8 && bytes_left > 0) {
        read_cluster(cluster, cluster_buf);

        uint32_t to_copy = (bytes_left < cluster_bytes) ? bytes_left : cluster_bytes;
        for (uint32_t i = 0; i < to_copy; i++) {
            dest[total_read + i] = cluster_buf[i];
        }

        total_read += to_copy;
        bytes_left -= to_copy;
        cluster = get_next_cluster(cluster);
    }

    return total_read;
}

void fat32_list_files(void) {
    if (!fs_ready) {
        print("Filesystem not available\n");
        return;
    }

    static uint8_t root_buffer[8192];
    uint32_t current_cluster = root_cluster;

    while (current_cluster >= 2 && current_cluster < 0x0FFFFFF8) {
        read_cluster(current_cluster, root_buffer);
        fat_entry_t* entries = (fat_entry_t*)root_buffer;

        uint32_t entries_per_cluster = fs_cluster_size() / 32;
        if (entries_per_cluster == 0) break;

        for (uint32_t i = 0; i < entries_per_cluster; i++) {
            if (entries[i].name[0] == 0x00) return;
            if (entries[i].name[0] == 0xE5) continue;
            if (entries[i].attributes == 0x0F) continue;
            if (entries[i].attributes & 0x08) continue;

            for (uint8_t c = 0; c < 8; c++) {
                if (entries[i].name[c] != ' ') print_char(entries[i].name[c], rgb(255, 255, 255));
            }
            if (entries[i].name[8] != ' ' || entries[i].name[9] != ' ' || entries[i].name[10] != ' ') {
                print_char('.', rgb(255, 255, 255));
                for (uint8_t c = 8; c < 11; c++) {
                    if (entries[i].name[c] != ' ') print_char(entries[i].name[c], rgb(255, 255, 255));
                }
            }
            if (entries[i].attributes & 0x10) print_char('/', rgb(255, 255, 255));
        }

        current_cluster = get_next_cluster(current_cluster);
    }
}
