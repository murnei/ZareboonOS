#include "stdint.h"
#include "stdbool.h"
#include "io.h"
#include "console.h"
#include "floppy.h"
#include "stdio.h"

#define FDC_DOR   0x3F2
#define FDC_MSR   0x3F4
#define FDC_FIFO  0x3F5
#define FDC_CCR   0x3F7

#define CMD_SPECIFY       0x03
#define CMD_READ_DATA     0xE6
#define CMD_RECALIBRATE   0x07
#define CMD_SENSE_INT     0x08
#define CMD_SEEK          0x0F

#define DMA_MASK_REG   0x0A
#define DMA_MODE_REG   0x0B
#define DMA_CLEAR_FF   0x0C
#define DMA_ADDR_CH2   0x04
#define DMA_COUNT_CH2  0x05
#define DMA_PAGE_CH2   0x81

#define DMA_BUFFER_ADDR 0x20000

static uint8_t* const dma_buffer = (uint8_t*)DMA_BUFFER_ADDR;

static bool fdc_ready = false;

static void floppy_wait_irq(void) {
    for (volatile uint32_t i = 0; i < 500000; i++) {
        if (inb(FDC_MSR) & 0x80) return;
    }
}

static void floppy_write_cmd(uint8_t cmd) {
    for (int i = 0; i < 10000; i++) {
        uint8_t msr = inb(FDC_MSR);
        if ((msr & 0xC0) == 0x80) {
            outb(FDC_FIFO, cmd);
            return;
        }
    }
}

static uint8_t floppy_read_data(void) {
    for (int i = 0; i < 10000; i++) {
        uint8_t msr = inb(FDC_MSR);
        if ((msr & 0xC0) == 0xC0) {
            return inb(FDC_FIFO);
        }
    }
    return 0xFF;
}

static void floppy_sense_interrupt(uint8_t* st0, uint8_t* cyl) {
    floppy_write_cmd(CMD_SENSE_INT);
    *st0 = floppy_read_data();
    *cyl = floppy_read_data();
}

static void dma_setup_read(uint32_t length) {
    outb(DMA_MASK_REG, 0x06);
    outb(DMA_CLEAR_FF, 0xFF);

    outb(DMA_MODE_REG, 0x46);

    uint32_t addr = DMA_BUFFER_ADDR;
    outb(DMA_ADDR_CH2, (uint8_t)(addr & 0xFF));
    outb(DMA_ADDR_CH2, (uint8_t)((addr >> 8) & 0xFF));
    outb(DMA_PAGE_CH2, (uint8_t)((addr >> 16) & 0xFF));

    uint32_t count = length - 1;
    outb(DMA_COUNT_CH2, (uint8_t)(count & 0xFF));
    outb(DMA_COUNT_CH2, (uint8_t)((count >> 8) & 0xFF));

    outb(DMA_MASK_REG, 0x02);
}

static bool floppy_recalibrate(void) {
    outb(FDC_DOR, 0x1C);

    floppy_write_cmd(CMD_RECALIBRATE);
    floppy_write_cmd(0x00);

    floppy_wait_irq();

    uint8_t st0, cyl;
    floppy_sense_interrupt(&st0, &cyl);

    return (st0 & 0xC0) == 0;
}

bool floppy_init(void) {
    outb(FDC_DOR, 0x00);
    for (volatile int i = 0; i < 10000; i++);
    outb(FDC_DOR, 0x1C);

    floppy_wait_irq();

    uint8_t st0, cyl;
    floppy_sense_interrupt(&st0, &cyl);

    floppy_write_cmd(CMD_SPECIFY);
    floppy_write_cmd(0xDF);
    floppy_write_cmd(0x02);

    if (!floppy_recalibrate()) {
        print("FDC_RECALIBRATE_FAIL");
        fdc_ready = false;
        return false;
    }

    fdc_ready = true;
    return true;
}

static void lba_to_chs(uint32_t lba, uint8_t* cyl, uint8_t* head, uint8_t* sector) {
    *cyl = (uint8_t)(lba / (2 * 18));
    *head = (uint8_t)((lba % (2 * 18)) / 18);
    *sector = (uint8_t)((lba % 18) + 1);
}

bool floppy_read_sector(uint32_t lba, uint8_t* buffer) {
    if (!fdc_ready) return false;

    uint8_t cyl, head, sector;
    lba_to_chs(lba, &cyl, &head, &sector);

    dma_setup_read(512);

    outb(FDC_CCR, 0x00);

    floppy_write_cmd(CMD_READ_DATA | 0x40);
    floppy_write_cmd((uint8_t)(head << 2));
    floppy_write_cmd(cyl);
    floppy_write_cmd(head);
    floppy_write_cmd(sector);
    floppy_write_cmd(0x02);
    floppy_write_cmd(18);
    floppy_write_cmd(0x1B);
    floppy_write_cmd(0xFF);

    floppy_wait_irq();

    uint8_t st0_status[7];
    for (int i = 0; i < 7; i++) st0_status[i] = floppy_read_data();

    uint8_t st0, cyl_after;
    floppy_sense_interrupt(&st0, &cyl_after);

#ifdef FLOPPY_DEBUG
    print("ST0=%x ST1=%x ST2=%x C=%x H=%x S=%x SZ=%x SENSE_ST0=%x DMA0=%x DMA1=%x",
        st0_status[0], st0_status[1], st0_status[2], st0_status[3],
        st0_status[4], st0_status[5], st0_status[6], st0,
        dma_buffer[0], dma_buffer[1]);
#endif

    for (int i = 0; i < 512; i++) {
        buffer[i] = dma_buffer[i];
    }

    return true;
}
