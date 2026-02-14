#include <kernel/pci.h>
#include <kernel/console.h>

static uint8_t pci_device_count = 0;
static pci_device_t pci_devices[64];

static uint8_t pci_read_device(uint8_t bus, uint8_t device, uint8_t func, pci_cfg_t *cfg)
{
    uint32_t vendor_device = pci_read32(pci_addr(bus, device, func, 0x0));
    if (vendor_device == 0xffffffff)
    {
        return 0;
    }
    uint32_t cmd_stat = pci_read32(pci_addr(bus, device, func, 0x4));
    uint32_t rev_cls = pci_read32(pci_addr(bus, device, func, 0x8));
    cfg->vendor_id = (uint16_t) (vendor_device & 0xffff);
    cfg->device_id = (uint16_t) (vendor_device >> 16);
    cfg->command = (uint16_t) (cmd_stat & 0xffff);
    cfg->status = (uint16_t) (cmd_stat >> 16);
    cfg->revision = (uint8_t) (rev_cls & 0xff);
    cfg->prog_if = (uint8_t) ((rev_cls >> 8) & 0xff);
    cfg->sub_class = (uint8_t) ((rev_cls >> 16) & 0xff);
    cfg->base_class = (uint8_t) ((rev_cls >> 24) & 0xff);
    for (uint8_t i = 0; i < 6; i++)
    {
        cfg->bar[i] = pci_read32(pci_addr(bus, device, func, (uint8_t) (0x10 + i * 4)));
    }
    return 1;
}

void scan_pci(void)
{
    console_puts("Scanning PCIs ...\n");
    for (uint16_t bus = 0; bus < 256; bus++)
    {
        for (uint8_t device = 0; device < 32; device++)
        {
            for (uint8_t func = 0; func < 8; func++)
            {
                pci_device_t dev;
                dev.bus = bus;
                dev.device = device;
                dev.func = func;
                if (!pci_read_device((uint8_t) bus, device, func, &dev.cfg))
                {
                    continue;
                }
                pci_devices[pci_device_count] = dev;
                pci_device_count++;
                console_puts("  ");
                console_put_hex8(dev.bus, 0);
                console_puts(":");
                console_put_hex8(dev.device, 0);
                console_puts(".");
                console_put_hex8(dev.func, 0);
                console_puts(", Vendor: ");
                console_put_hex16(dev.cfg.vendor_id, 1);
                console_puts(", Device: ");
                console_put_hex16(dev.cfg.device_id, 1);
                console_puts(", Class code: ");
                console_put_hex8(dev.cfg.base_class, 1);
                console_put_hex8(dev.cfg.sub_class, 0);
                console_put_hex8(dev.cfg.prog_if, 0);
                console_puts("\n");
            }
        }
    }
    console_puts("Done\n");
}

uint8_t pci_find_device(uint8_t base_class, uint8_t sub_class, pci_device_t *out)
{
    for (uint8_t i = 0; i < pci_device_count; i++)
    {
        pci_device_t *dev = &pci_devices[i];
        if (dev->cfg.base_class == base_class && dev->cfg.sub_class == sub_class)
        {
            *out = *dev;
            return 1;
        }
    }
    return 0;
}

void pci_enable_bus_master(pci_device_t *dev)
{
    if (dev->cfg.command & 0x4)
    {
        return;
    }
    dev->cfg.command |= 0x4;
    pci_write16(pci_addr(dev->bus, dev->device, dev->func, 0x4), dev->cfg.command);
}
