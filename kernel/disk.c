#include <stdint.h>
#include <string.h>
#include <kernel/disk.h>
#include <kernel/io.h>
#include <kernel/pci.h>
#include <kernel/console.h>
#include <kernel/interrupts.h>
#include <kernel/timer.h>

static prdt_entry_t primary_prdt __attribute__((aligned(4)));
static uint8_t dma_buffer[DISK_DMA_MAX_SECTORS * DISK_SECTOR_SIZE] __attribute__((aligned(16)));
static pci_device_t ide_dev;

static void ata_irq_handler(void)
{
    inb(ATA_PRIMARY_DATA_REGISTER + ATA_STATUS_REGISTER_OFFSET);
}

uint8_t init_disk(void)
{
    console_puts("Initializing IDE disk ... ");
    if (!pci_find_device(0x1, 0x1, &ide_dev))
    {
        console_puts("Failed: IDE controller not found\n");
        return 0;
    }
    if ((ide_dev.cfg.bar[4] & 0xfffc) == 0)
    {
        console_puts("Failed: bus-master BAR missing\n");
        return 0;
    }
    pci_enable_bus_master(&ide_dev);
    outb(ATA_PRIMARY_CONTROL_REGISTER, 0x0);
    register_irq_handler(14, ata_irq_handler);
    unmask_irq(14);
    console_puts("Done\n");
    return 1;
}

static void ata_io_wait(void)
{
    inb(ATA_PRIMARY_CONTROL_REGISTER);
    inb(ATA_PRIMARY_CONTROL_REGISTER);
    inb(ATA_PRIMARY_CONTROL_REGISTER);
    inb(ATA_PRIMARY_CONTROL_REGISTER);
}

static uint8_t ata_wait4_not_busy(void)
{
    uint32_t start = get_current_ticks();
    while ((uint32_t)(get_current_ticks() - start) < ATA_BUSY_TIMEOUT_TICKS)
    {
        uint8_t ata_status = inb(ATA_PRIMARY_DATA_REGISTER + ATA_STATUS_REGISTER_OFFSET);
        if (!(ata_status & ATA_STATUS_BUSY))
        {
            return 1;
        }
        ata_io_wait();
    }
    return 0;
}

static void ata_set_lba28(uint32_t lba)
{
    outb(ATA_PRIMARY_DATA_REGISTER + ATA_DRVHEAD_REGISTER_OFFSET, (uint8_t) (0xe0 | ((lba >> 24) & 0xf)));
    ata_io_wait();
}

static uint8_t ata_wait4_dma(void)
{
    uint32_t start = get_current_ticks();
    while ((uint32_t) (get_current_ticks() - start) < ATA_DMA_TIMEOUT_TICKS)
    {
        uint8_t ata_status = inb(ATA_PRIMARY_DATA_REGISTER + ATA_STATUS_REGISTER_OFFSET);
        uint8_t bm_status = inb((ide_dev.cfg.bar[4] & 0xfffc) + BM_STATUS_REGISTER_OFFSET);
        if (ata_status & (ATA_STATUS_ERROR | ATA_STATUS_FAULT))
        {
            return 0;
        }
        if (bm_status & BM_STATUS_ERROR)
        {
            return 0;
        }
        if (bm_status & BM_STATUS_IRQ_COMPLETED)
        {
            return 1;
        }
    }
    return 0;
}

static uint8_t ata_dma_transfer(uint32_t lba, uint8_t sectors, uint8_t is_write)
{
    if (sectors == 0 || sectors > DISK_DMA_MAX_SECTORS)
    {
        return 0;
    }
    primary_prdt.buffer_phys_addr = (uint32_t) dma_buffer;
	primary_prdt.transfer_bytes = (uint16_t) (sectors * DISK_SECTOR_SIZE);
	primary_prdt.reserved = PRDT_END;
    if (!ata_wait4_not_busy())
    {
        return 0;
    }
    uint16_t bm_base = ide_dev.cfg.bar[4] & 0xfffc;
    uint16_t bm_command_port = (uint16_t) (bm_base + BM_COMMAND_REGISTER_OFFSET);
    uint8_t bm_direction = is_write ? BM_COMMAND_WRITE : BM_COMMAND_READ;
    uint16_t bm_status_port = (uint16_t) (bm_base + BM_STATUS_REGISTER_OFFSET);
    uint16_t bm_prdt_port = (uint16_t) (bm_base + BM_PRDT_REGISTER_OFFSET);
    outb(bm_command_port, (uint8_t) (inb(bm_command_port) & ~BM_COMMAND_START));
    outb(bm_command_port, bm_direction);
    outb(bm_status_port, (uint8_t) (BM_STATUS_IRQ_COMPLETED | BM_STATUS_ERROR));
	outl(bm_prdt_port, (uint32_t) &primary_prdt);
    ata_set_lba28(lba);
    outb(ATA_PRIMARY_DATA_REGISTER + ATA_FEATURE_REGISTER_OFFSET, 0x0);
    outb(ATA_PRIMARY_DATA_REGISTER + ATA_SECCOUNT_REGISTER_OFFSET, sectors);
    outb(ATA_PRIMARY_DATA_REGISTER + ATA_LBALOW_REGISTER_OFFSET, (uint8_t) lba);
    outb(ATA_PRIMARY_DATA_REGISTER + ATA_LBAMID_REGISTER_OFFSET, (uint8_t) (lba >> 8));
    outb(ATA_PRIMARY_DATA_REGISTER + ATA_LBAHIGHT_REGISTER_OFFSET, (uint8_t) (lba >> 16));
    outb(bm_command_port, (uint8_t) (bm_direction | BM_COMMAND_START));
    uint8_t ata_command = is_write ? ATA_COMMAND_DMA_WRITE : ATA_COMMAND_DMA_READ;
    outb(ATA_PRIMARY_DATA_REGISTER + ATA_COMMAND_REGISTER_OFFSET, ata_command);
    if (!ata_wait4_dma())
    {
        outb(bm_command_port, bm_direction);
        return 0;
    }
    outb(bm_command_port, bm_direction);
    if (!ata_wait4_not_busy())
    {
        return 0;
    }
    uint8_t status = inb(ATA_PRIMARY_DATA_REGISTER + ATA_STATUS_REGISTER_OFFSET);
    uint8_t bm_status = inb(bm_status_port);
    outb(bm_status_port, BM_STATUS_IRQ_COMPLETED | BM_STATUS_ERROR);
    if ((status & (ATA_STATUS_ERROR | ATA_STATUS_FAULT)) || (bm_status & BM_STATUS_ERROR))
    {
        return 0;
    }
    if (is_write)
    {
        outb(ATA_PRIMARY_DATA_REGISTER + ATA_COMMAND_REGISTER_OFFSET, ATA_COMMAND_DMA_CACHE_FLUSH);
        if (!ata_wait4_not_busy())
        {
            return 0;
        }
        if (inb(ATA_PRIMARY_DATA_REGISTER + ATA_STATUS_REGISTER_OFFSET) & (ATA_STATUS_ERROR | ATA_STATUS_FAULT))
        {
            return 0;
        }
    }
    return 1;
}

uint8_t disk_read(uint32_t lba, void *buffer, uint8_t sectors)
{
    uint8_t *out = (uint8_t *) buffer;
    uint32_t current_lba = lba;
    uint8_t remaining = sectors;
    while (remaining)
    {
        uint8_t chunk = remaining;
        if (chunk > DISK_DMA_MAX_SECTORS)
        {
            chunk = DISK_DMA_MAX_SECTORS;
        }
        uint16_t bytes = (uint16_t) (chunk * DISK_SECTOR_SIZE);
        if (!ata_dma_transfer(current_lba, chunk, 0))
        {
            return 0;
        }
        memcpy(out, dma_buffer, bytes);
        out += bytes;
        current_lba += chunk;
        remaining -= chunk;
    }
    return 1;
}

uint8_t disk_write(uint32_t lba, const void *buffer, uint8_t sectors)
{
    const uint8_t *in = (const uint8_t *) buffer;
    uint32_t current_lba = lba;
    uint8_t remaining = sectors;
    while (remaining)
    {
        uint8_t chunk = remaining;
        if (chunk > DISK_DMA_MAX_SECTORS)
        {
            chunk = DISK_DMA_MAX_SECTORS;
        }
        uint16_t bytes = (uint16_t) (chunk * DISK_SECTOR_SIZE);
        memcpy(dma_buffer, in, bytes);
        if (!ata_dma_transfer(current_lba, chunk, 1))
        {
            return 0;
        }
        in += bytes;
        current_lba += chunk;
        remaining -= chunk;
    }
    return 1;
}
