#include "stdint.h"
#include "stdbool.h"
#include "ata.h"

#define ATA_SR_ERR  0x01
#define ATA_SR_DRQ  0x08
#define ATA_SR_DF   0x20
#define ATA_SR_BSY  0x80

#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_IDENTIFY 0xEC

#define ATA_CMD_WRITE_PIO   0x30
#define ATA_CMD_CACHE_FLUSH 0xE7

uint8_t ata_last_status = 0;
uint8_t ata_last_error  = 0;
int     ata_found_index = -1;

static uint16_t io_base   = 0x1F0;
static uint16_t ctrl_base = 0x3F6;
static uint8_t  drive_bit = 0;
static bool     ata_present = false;

static inline void ata_outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t ata_inb(uint16_t port) {
    uint8_t v;
    __asm__ volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

static inline void ata_insw(uint16_t port, void* addr, uint32_t count) {
    __asm__ volatile ("cld; rep insw"
                      : "+D"(addr), "+c"(count)
                      : "d"(port)
                      : "memory");
}

static inline void ata_outsw(uint16_t port, const void* addr, uint32_t count) {
    __asm__ volatile ("cld; rep outsw"
                      : "+S"(addr), "+c"(count)
                      : "d"(port)
                      : "memory");
}

static void ata_delay_400ns(uint16_t ctrl) {
    for (int i = 0; i < 4; i++) ata_inb(ctrl);
}

static bool ata_probe(uint16_t base, uint16_t ctrl, uint8_t slave) {
    ata_outb(base + 6, 0xA0 | (slave << 4));
    ata_delay_400ns(ctrl);

    ata_outb(base + 2, 0);
    ata_outb(base + 3, 0);
    ata_outb(base + 4, 0);
    ata_outb(base + 5, 0);
    ata_outb(base + 7, ATA_CMD_IDENTIFY);
    ata_delay_400ns(ctrl);

    uint8_t st = ata_inb(base + 7);
    if (st == 0x00 || st == 0xFF) return false;

    uint32_t timeout = 1000000;
    while ((ata_inb(base + 7) & ATA_SR_BSY) && --timeout);
    if (timeout == 0) return false;

    if (ata_inb(base + 4) != 0 || ata_inb(base + 5) != 0) return false;

    timeout = 1000000;
    do {
        st = ata_inb(base + 7);
        if (st & (ATA_SR_ERR | ATA_SR_DF)) return false;
        if (st & ATA_SR_DRQ) break;
    } while (--timeout);
    if (timeout == 0) return false;

    uint16_t junk[256];
    ata_insw(base, junk, 256);
    return true;
}

void ata_init(void) {
    ata_outb(0x3F6, 0x02);
    ata_outb(0x376, 0x02);

    static const uint16_t bases[2] = { 0x1F0, 0x170 };
    static const uint16_t ctrls[2] = { 0x3F6, 0x376 };

    ata_present = false;
    ata_found_index = -1;

    for (int bus = 0; bus < 2 && !ata_present; bus++) {
        for (int slave = 0; slave < 2; slave++) {
            if (ata_probe(bases[bus], ctrls[bus], (uint8_t)slave)) {
                io_base   = bases[bus];
                ctrl_base = ctrls[bus];
                drive_bit = (uint8_t)slave;
                ata_found_index = bus * 2 + slave;
                ata_present = true;
                break;
            }
        }
    }
}

bool ata_read_sector(uint32_t lba, void* buffer) {
    if (!ata_present) {
        ata_last_status = 0xEE;
        ata_last_error  = 0;
        return false;
    }

    uint32_t timeout;

    timeout = 1000000;
    while ((ata_inb(io_base + 7) & ATA_SR_BSY) && --timeout);
    if (timeout == 0) {
        ata_last_status = ata_inb(io_base + 7);
        ata_last_error  = ata_inb(io_base + 1);
        return false;
    }

    ata_outb(io_base + 6, 0xE0 | (drive_bit << 4) | ((lba >> 24) & 0x0F));
    ata_delay_400ns(ctrl_base);

    ata_outb(io_base + 2, 1);
    ata_outb(io_base + 3, (uint8_t)(lba));
    ata_outb(io_base + 4, (uint8_t)(lba >> 8));
    ata_outb(io_base + 5, (uint8_t)(lba >> 16));
    ata_outb(io_base + 7, ATA_CMD_READ_PIO);

    ata_delay_400ns(ctrl_base);

    timeout = 1000000;
    uint8_t status;
    do {
        status = ata_inb(io_base + 7);
        if (status & (ATA_SR_ERR | ATA_SR_DF)) {
            ata_last_status = status;
            ata_last_error  = ata_inb(io_base + 1);
            return false;
        }
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break;
    } while (--timeout);
    if (timeout == 0) {
        ata_last_status = status;
        ata_last_error  = ata_inb(io_base + 1);
        return false;
    }

    ata_insw(io_base, buffer, 256);
    return true;
}

bool ata_write_sector(uint32_t lba, const void* buffer) {
    if (!ata_present) {
        ata_last_status = 0xEE;
        ata_last_error  = 0;
        return false;
    }

    uint32_t timeout;
    uint8_t status;

    timeout = 1000000;
    while ((ata_inb(io_base + 7) & ATA_SR_BSY) && --timeout);
    if (timeout == 0) {
        ata_last_status = ata_inb(io_base + 7);
        ata_last_error  = ata_inb(io_base + 1);
        return false;
    }

    ata_outb(io_base + 6, 0xE0 | (drive_bit << 4) | ((lba >> 24) & 0x0F));
    ata_delay_400ns(ctrl_base);

    ata_outb(io_base + 2, 1);
    ata_outb(io_base + 3, (uint8_t)(lba));
    ata_outb(io_base + 4, (uint8_t)(lba >> 8));
    ata_outb(io_base + 5, (uint8_t)(lba >> 16));
    ata_outb(io_base + 7, ATA_CMD_WRITE_PIO);

    ata_delay_400ns(ctrl_base);

    timeout = 1000000;
    do {
        status = ata_inb(io_base + 7);
        if (status & (ATA_SR_ERR | ATA_SR_DF)) {
            ata_last_status = status;
            ata_last_error  = ata_inb(io_base + 1);
            return false;
        }
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break;
    } while (--timeout);
    if (timeout == 0) {
        ata_last_status = status;
        ata_last_error  = ata_inb(io_base + 1);
        return false;
    }

    ata_outsw(io_base, buffer, 256);

    ata_outb(io_base + 7, ATA_CMD_CACHE_FLUSH);

    timeout = 1000000;
    while ((ata_inb(io_base + 7) & ATA_SR_BSY) && --timeout);
    status = ata_inb(io_base + 7);
    if (timeout == 0 || (status & (ATA_SR_ERR | ATA_SR_DF))) {
        ata_last_status = status;
        ata_last_error  = ata_inb(io_base + 1);
        return false;
    }

    return true;
}
