#include "stdint.h"
#include "stdbool.h"
#include "io.h"
#include "console.h"
#include "stdio.h"
#include "video.h"
#include "fs.h"
#include "ata.h"

static uint16_t bytes_per_sec = 0;
static uint8_t  sec_per_clus = 0;
static uint16_t reserved_sectors = 0;
static uint8_t  num_fats = 0;
static uint32_t fat_size_32 = 0;
static uint32_t root_cluster = 0;
static uint32_t data_sec = 0;
static uint32_t total_clusters = 0;
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
    ata_init();
    partition_lba = FS_PARTITION_LBA;

    if (!ata_read_sector(partition_lba, sector_buffer)) {
        print("\nATA_READ_FAILED st=%d err=%d\n", (int)ata_last_status, (int)ata_last_error);
        fs_ready = false;
        return;
    }

    bytes_per_sec    = *(uint16_t*)(sector_buffer + 0x0B);
    sec_per_clus     = *(uint8_t*)(sector_buffer + 0x0D);
    reserved_sectors = *(uint16_t*)(sector_buffer + 0x0E);
    num_fats         = *(uint8_t*)(sector_buffer + 0x10);
    fat_size_32      = *(uint32_t*)(sector_buffer + 0x24);
    root_cluster     = *(uint32_t*)(sector_buffer + 0x2C);

    if (bytes_per_sec != 512 || sec_per_clus == 0) {
        print("\nFS_INIT_FAILED bps=%d spc=%d b0=%d\n", (int)bytes_per_sec, (int)sec_per_clus, (int)sector_buffer[0]);
        fs_ready = false;
        return;
    }

    uint32_t total_sectors = *(uint16_t*)(sector_buffer + 0x13);
    if (total_sectors == 0)
        total_sectors = *(uint32_t*)(sector_buffer + 0x20);

    data_sec = reserved_sectors + (num_fats * fat_size_32);
    total_clusters = (total_sectors - data_sec) / sec_per_clus;

    fs_ready = true;
}

uint32_t fs_cluster_size(void) {
    return (uint32_t)bytes_per_sec * (uint32_t)sec_per_clus;
}

static uint32_t calculate_LBA(uint32_t cluster) {
    return partition_lba + data_sec + (cluster - 2) * sec_per_clus;
}

static void read_cluster(uint32_t cluster, uint8_t* buffer) {
    if (!fs_ready) return;

    uint32_t start_lba = calculate_LBA(cluster);
    for (uint8_t i = 0; i < sec_per_clus; i++) {
        ata_read_sector(start_lba + i, buffer + (i * 512));
    }
}

static bool write_cluster(uint32_t cluster, const uint8_t* buffer) {
    if (!fs_ready) return false;

    uint32_t start_lba = calculate_LBA(cluster);
    for (uint8_t i = 0; i < sec_per_clus; i++) {
        if (!ata_write_sector(start_lba + i, buffer + (i * 512))) return false;
    }
    return true;
}

static uint32_t get_next_cluster(uint32_t cluster) {
    if (!fs_ready || bytes_per_sec == 0) {
        return 0x0FFFFFFF;
    }

    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = partition_lba + reserved_sectors + (fat_offset / bytes_per_sec);
    uint32_t ent_offset = fat_offset % bytes_per_sec;

    static uint8_t fat_buffer[512];
    ata_read_sector(fat_sector, fat_buffer);

    uint32_t next_cluster = *(uint32_t*)&fat_buffer[ent_offset];
    return next_cluster & 0x0FFFFFFF;
}

static uint32_t find_free_cluster(void) {
    if (!fs_ready || bytes_per_sec == 0) return 0x0FFFFFFF;

    static uint8_t fat_buf[512];
    uint32_t entries_per_sector = bytes_per_sec / 4;
    uint32_t max_cluster = total_clusters + 2;

    for (uint32_t s = 0; s < fat_size_32; s++) {
        if (!ata_read_sector(partition_lba + reserved_sectors + s, fat_buf))
            return 0x0FFFFFFF;

        uint32_t* fat = (uint32_t*)fat_buf;

        for (uint32_t i = 0; i < entries_per_sector; i++) {
            uint32_t cluster = s * entries_per_sector + i;

            if (cluster < 2) continue;
            if (cluster >= max_cluster) return 0x0FFFFFFF;

            if ((fat[i] & 0x0FFFFFFF) == 0) {
                return cluster;
            }
        }
    }

    return 0x0FFFFFFF;
}

static bool fat_set_entry(uint32_t cluster, uint32_t value) {
    if (!fs_ready) return false;

    static uint8_t fat_buf[512];
    uint32_t fat_offset = cluster * 4;
    uint32_t sector_in_fat = fat_offset / bytes_per_sec;
    uint32_t ent_offset = fat_offset % bytes_per_sec;

    for (uint8_t f = 0; f < num_fats; f++) {
        uint32_t lba = partition_lba + reserved_sectors + f * fat_size_32 + sector_in_fat;

        if (!ata_read_sector(lba, fat_buf)) return false;

        uint32_t* entry = (uint32_t*)&fat_buf[ent_offset];
        *entry = (*entry & 0xF0000000) | (value & 0x0FFFFFFF);

        if (!ata_write_sector(lba, fat_buf)) return false;
    }
    return true;
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
        print("\nFilesystem not available\n");
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
        print("\nFilesystem not available");
        return;
    }

    static uint8_t root_buffer[16384];
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
            print_char('\n', rgb(0, 0, 0));
        }

        current_cluster = get_next_cluster(current_cluster);
    }
}

void make_dir(uint8_t* name) {
    static uint8_t buff[16384];

    if (!fs_ready) return;

    uint32_t new_clus = find_free_cluster();
    if (new_clus == 0x0FFFFFFF) {
        print("\nNo free clusters\n");
        return;
    }

    if (!fat_set_entry(new_clus, 0x0FFFFFFF)) {
        print("\nFAT write failed\n");
        return;
    }

    uint32_t cluster_bytes = fs_cluster_size();
    for (uint32_t i = 0; i < cluster_bytes; i++) buff[i] = 0;

    fat_entry_t* dots = (fat_entry_t*)buff;

    for (int i = 0; i < 11; i++) { dots[0].name[i] = ' '; dots[1].name[i] = ' '; }
    dots[0].name[0] = '.';
    dots[1].name[0] = '.'; dots[1].name[1] = '.';

    dots[0].attributes = 0x10;
    dots[0].cluster_high = (uint16_t)(new_clus >> 16);
    dots[0].cluster_low  = (uint16_t)(new_clus & 0xFFFF);

    dots[1].attributes = 0x10;
    dots[1].cluster_high = 0;
    dots[1].cluster_low  = 0;

    write_cluster(new_clus, buff);

    uint32_t dir_clus = root_cluster;
    uint32_t per_clus = cluster_bytes / 32;

    while (dir_clus >= 2 && dir_clus < 0x0FFFFFF8) {
        read_cluster(dir_clus, buff);
        fat_entry_t* entries = (fat_entry_t*)buff;

        for (uint32_t i = 0; i < per_clus; i++) {
            if (entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) {
                uint8_t* raw = (uint8_t*)&entries[i];
                for (int b = 0; b < 32; b++) raw[b] = 0;

                format_fat_name(name, entries[i].name);
                entries[i].attributes   = 0x10;
                entries[i].cluster_high = (uint16_t)(new_clus >> 16);
                entries[i].cluster_low  = (uint16_t)(new_clus & 0xFFFF);
                entries[i].file_size    = 0;

                write_cluster(dir_clus, buff);
                return;
            }
        }
        dir_clus = get_next_cluster(dir_clus);
    }

    fat_set_entry(new_clus, 0);
    print("\nDirectory is full\n");
}

static bool is_dot_entry(const fat_entry_t* e) {
    if (e->name[0] != '.') return false;
    if (e->name[1] == ' ') return true;
    if (e->name[1] == '.' && e->name[2] == ' ') return true;
    return false;
}

static bool dir_is_empty(uint32_t start_clus) {
    static uint8_t dbuf[16384];
    uint32_t clus = start_clus;
    uint32_t per_clus = fs_cluster_size() / 32;

    while (clus >= 2 && clus < 0x0FFFFFF8) {
        read_cluster(clus, dbuf);
        fat_entry_t* e = (fat_entry_t*)dbuf;

        for (uint32_t i = 0; i < per_clus; i++) {
            if (e[i].name[0] == 0x00) return true;
            if (e[i].name[0] == 0xE5) continue;
            if (e[i].attributes == 0x0F) continue;
            if (is_dot_entry(&e[i])) continue;
            return false;
        }
        clus = get_next_cluster(clus);
    }
    return true;
}

static void free_cluster_chain(uint32_t clus) {
    while (clus >= 2 && clus < 0x0FFFFFF8) {
        uint32_t next = get_next_cluster(clus);
        fat_set_entry(clus, 0);
        clus = next;
    }
}

void remove_dir(uint8_t* name) {
    static uint8_t buff[16384];

    if (!fs_ready) return;

    uint8_t target[11];
    format_fat_name(name, target);

    uint32_t dir_clus = root_cluster;
    uint32_t per_clus = fs_cluster_size() / 32;

    while (dir_clus >= 2 && dir_clus < 0x0FFFFFF8) {
        read_cluster(dir_clus, buff);
        fat_entry_t* entries = (fat_entry_t*)buff;

        for (uint32_t i = 0; i < per_clus; i++) {
            if (entries[i].name[0] == 0x00) {
                print("\nDirectory not found\n");
                return;
            }
            if (entries[i].name[0] == 0xE5) continue;
            if (entries[i].attributes == 0x0F) continue;
            if (entries[i].attributes & 0x08) continue;
            if (!fat_name_equal(entries[i].name, target)) continue;

            if (!(entries[i].attributes & 0x10)) {
                print("\nNot a directory\n");
                return;
            }

            uint32_t clus = ((uint32_t)entries[i].cluster_high << 16) | entries[i].cluster_low;

            if (!dir_is_empty(clus)) {
                print("\nDirectory not empty\n");
                return;
            }

            entries[i].name[0] = 0xE5;
            write_cluster(dir_clus, buff);

            free_cluster_chain(clus);
            return;
        }
        dir_clus = get_next_cluster(dir_clus);
    }

    print("\nDirectory not found\n");
}


typedef struct {
    bool     found;
    uint32_t clus;
    uint32_t idx;
} dir_pos_t;

static uint8_t io_buf[16384];

static void scan_root(const uint8_t target[11], dir_pos_t* hit, dir_pos_t* slot, fat_entry_t* hit_entry) {
    hit->found = false;
    slot->found = false;

    uint32_t clus = root_cluster;
    uint32_t per_clus = fs_cluster_size() / 32;

    while (clus >= 2 && clus < 0x0FFFFFF8) {
        read_cluster(clus, io_buf);
        fat_entry_t* e = (fat_entry_t*)io_buf;

        for (uint32_t i = 0; i < per_clus; i++) {
            uint8_t first = e[i].name[0];

            if (first == 0x00 || first == 0xE5) {
                if (!slot->found) {
                    slot->found = true;
                    slot->clus = clus;
                    slot->idx = i;
                }
                if (first == 0x00) return;
                continue;
            }
            if (e[i].attributes == 0x0F) continue;
            if (e[i].attributes & 0x08) continue;

            if (fat_name_equal(e[i].name, target)) {
                hit->found = true;
                hit->clus = clus;
                hit->idx = i;
                *hit_entry = e[i];
                return;
            }
        }
        clus = get_next_cluster(clus);
    }
}

static bool store_entry(const dir_pos_t* pos, const uint8_t* name11, uint32_t first, uint32_t size) {
    read_cluster(pos->clus, io_buf);
    fat_entry_t* e = &((fat_entry_t*)io_buf)[pos->idx];

    if (name11) {
        uint8_t* raw = (uint8_t*)e;
        for (int b = 0; b < 32; b++) raw[b] = 0;
        for (int b = 0; b < 11; b++) e->name[b] = name11[b];
        e->attributes = 0x20;
    }

    e->cluster_high = (uint16_t)(first >> 16);
    e->cluster_low  = (uint16_t)(first & 0xFFFF);
    e->file_size    = size;

    return write_cluster(pos->clus, io_buf);
}

static int alloc_and_write(uint32_t prev, const uint8_t* data, uint32_t size, uint32_t* first_out) {
    uint32_t cb = fs_cluster_size();
    *first_out = 0;

    while (size > 0) {
        uint32_t c = find_free_cluster();
        if (c == 0x0FFFFFFF) return FS_ERR_NOSPACE;

        if (!fat_set_entry(c, 0x0FFFFFFF)) return FS_ERR_IO;

        if (prev != 0) {
            if (!fat_set_entry(prev, c)) {
                fat_set_entry(c, 0);
                return FS_ERR_IO;
            }
        }
        if (*first_out == 0) *first_out = c;

        uint32_t n = (size < cb) ? size : cb;
        for (uint32_t i = 0; i < cb; i++) io_buf[i] = (i < n) ? data[i] : 0;

        if (!write_cluster(c, io_buf)) return FS_ERR_IO;

        data += n;
        size -= n;
        prev = c;
    }
    return FS_OK;
}

int fs_create_file(const uint8_t* name) {
    if (!fs_ready) return FS_ERR_NOFS;

    uint8_t target[11];
    format_fat_name(name, target);

    dir_pos_t hit, slot;
    fat_entry_t he;
    scan_root(target, &hit, &slot, &he);

    if (hit.found) return FS_ERR_EXISTS;
    if (!slot.found) return FS_ERR_DIRFULL;

    return store_entry(&slot, target, 0, 0) ? FS_OK : FS_ERR_IO;
}

int fs_write_file(const uint8_t* name, const uint8_t* data, uint32_t size) {
    if (!fs_ready) return FS_ERR_NOFS;

    uint8_t target[11];
    format_fat_name(name, target);

    dir_pos_t hit, slot, pos;
    fat_entry_t he;
    scan_root(target, &hit, &slot, &he);

    if (hit.found) {
        if (he.attributes & 0x10) return FS_ERR_ISDIR;

        pos = hit;
        uint32_t old = ((uint32_t)he.cluster_high << 16) | he.cluster_low;

        if (!store_entry(&pos, 0, 0, 0)) return FS_ERR_IO;
        free_cluster_chain(old);
    } else {
        if (!slot.found) return FS_ERR_DIRFULL;

        pos = slot;
        if (!store_entry(&pos, target, 0, 0)) return FS_ERR_IO;
    }

    uint32_t first = 0;
    int r = alloc_and_write(0, data, size, &first);
    if (r != FS_OK) {
        free_cluster_chain(first);
        return r;
    }

    return store_entry(&pos, 0, first, size) ? FS_OK : FS_ERR_IO;
}

int fs_append_file(const uint8_t* name, const uint8_t* data, uint32_t size) {
    if (!fs_ready) return FS_ERR_NOFS;

    uint8_t target[11];
    format_fat_name(name, target);

    dir_pos_t hit, slot;
    fat_entry_t he;
    scan_root(target, &hit, &slot, &he);

    if (!hit.found) return fs_write_file(name, data, size);
    if (he.attributes & 0x10) return FS_ERR_ISDIR;
    if (size == 0) return FS_OK;

    uint32_t first = ((uint32_t)he.cluster_high << 16) | he.cluster_low;
    if (first == 0) return fs_write_file(name, data, size);

    uint32_t cb = fs_cluster_size();
    uint32_t old_size = he.file_size;

    uint32_t last = first, count = 1;
    for (;;) {
        uint32_t next = get_next_cluster(last);
        if (next < 2 || next >= 0x0FFFFFF8) break;
        last = next;
        count++;
    }

    uint32_t before_last = (count - 1) * cb;
    uint32_t used = (old_size > before_last) ? (old_size - before_last) : 0;
    if (used > cb) used = cb;

    uint32_t fill = cb - used;
    if (fill > size) fill = size;

    if (fill > 0) {
        read_cluster(last, io_buf);
        for (uint32_t i = 0; i < fill; i++) io_buf[used + i] = data[i];
        if (!write_cluster(last, io_buf)) return FS_ERR_IO;
    }

    uint32_t rest = size - fill;
    if (rest > 0) {
        uint32_t new_first = 0;
        int r = alloc_and_write(last, data + fill, rest, &new_first);
        if (r != FS_OK) {
            fat_set_entry(last, 0x0FFFFFFF);
            free_cluster_chain(new_first);
            return r;
        }
    }

    return store_entry(&hit, 0, first, old_size + size) ? FS_OK : FS_ERR_IO;
}

int fs_cat(const uint8_t* name) {
    if (!fs_ready) return FS_ERR_NOFS;

    file_info_t info;
    if (!fs_find_file(name, &info)) return FS_ERR_NOTFOUND;

    uint32_t cluster = info.start_cluster;
    uint32_t left = info.file_size;
    uint32_t cb = fs_cluster_size();
    uint8_t last = '\n';

    while (cluster >= 2 && cluster < 0x0FFFFFF8 && left > 0) {
        read_cluster(cluster, io_buf);
        uint32_t n = (left < cb) ? left : cb;

        for (uint32_t i = 0; i < n; i++) {
            uint8_t c = io_buf[i];
            last = c;

            if (c == '\r') continue;
            if (c == '\t') c = ' ';
            if (c != '\n' && (c < 32 || c > 126)) c = '.';

            print_char(c, rgb(255, 255, 255));
        }

        left -= n;
        cluster = get_next_cluster(cluster);
    }

    if (last != '\n') print_char('\n', rgb(255, 255, 255));
    return FS_OK;
}
