#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define DISK_SECTOR_SIZE 512
#define DISK_DMA_MAX_SECTORS 8
#define ATA_PRIMARY_CONTROL_REGISTER 0x3f6
#define ATA_PRIMARY_DATA_REGISTER 0x1f0
#define ATA_DATA_REGISTER_OFFSET 0x0
#define ATA_ERROR_REGISTER_OFFSET 0x1
#define ATA_FEATURE_REGISTER_OFFSET 0x1
#define ATA_SECCOUNT_REGISTER_OFFSET 0x2
#define ATA_LBALOW_REGISTER_OFFSET 0x3
#define ATA_LBAMID_REGISTER_OFFSET 0x4
#define ATA_LBAHIGHT_REGISTER_OFFSET 0x5
#define ATA_DRVHEAD_REGISTER_OFFSET 0x6
#define ATA_COMMAND_REGISTER_OFFSET 0x7
#define ATA_STATUS_REGISTER_OFFSET 0x7
#define BM_COMMAND_REGISTER_OFFSET 0x0
#define BM_STATUS_REGISTER_OFFSET 0x2
#define BM_PRDT_REGISTER_OFFSET 0x4
#define ATA_COMMAND_DMA_READ 0xc8
#define ATA_COMMAND_DMA_WRITE 0xca
#define ATA_COMMAND_DMA_CACHE_FLUSH 0xe7
#define ATA_STATUS_ERROR 0x1
#define ATA_STATUS_DRQ 0x8
#define ATA_STATUS_FAULT 0x20
#define ATA_STATUS_READY 0x40
#define ATA_STATUS_BUSY 0x80
#define BM_COMMAND_START 0x1
#define BM_COMMAND_READ 0x8
#define BM_COMMAND_WRITE 0x0
#define BM_STATUS_ACTIVE 0x1
#define BM_STATUS_ERROR 0x2
#define BM_STATUS_IRQ_COMPLETED 0x4
#define PRDT_END 0x8000
#define ATA_BUSY_TIMEOUT_TICKS 100
#define ATA_DMA_TIMEOUT_TICKS 100

typedef struct __attribute__((packed))
{
    uint32_t buffer_phys_addr;
    uint16_t transfer_bytes;
    uint16_t reserved;
} prdt_entry_t;

uint8_t init_disk(void);
uint8_t disk_read(uint32_t lba, void *buffer, uint8_t sectors);
uint8_t disk_write(uint32_t lba, const void *buffer, uint8_t sectors);

#endif
