#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include <kernel/io.h>

#define PCI_CFG_ADDR_REGISTER 0xcf8
#define PCI_CFG_DATA_REGISTER 0xcfc

typedef struct
{
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision;
    uint8_t prog_if;
    uint8_t sub_class;
    uint8_t base_class;
    uint32_t bar[6];
} pci_cfg_t;

typedef struct
{
    uint16_t bus;
    uint8_t device;
    uint8_t func;
    pci_cfg_t cfg;
} pci_device_t;

static inline uint32_t pci_addr(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset)
{
    return (uint32_t) (
        0x80000000 |
        ((uint32_t) bus << 16) |
        ((uint32_t) device << 11) |
        ((uint32_t) func << 8) |
        (offset & 0xfc)
    );
}
static inline uint16_t pci_read16(uint32_t addr)
{
    outl(PCI_CFG_ADDR_REGISTER, addr);
    return inw(PCI_CFG_DATA_REGISTER + (addr & 0x2));
}
static inline void pci_write16(uint32_t addr, uint16_t val)
{
    outl(PCI_CFG_ADDR_REGISTER, addr);
    outw(PCI_CFG_DATA_REGISTER + (addr & 0x2), val);
}
static inline uint32_t pci_read32(uint32_t addr)
{
    outl(PCI_CFG_ADDR_REGISTER, addr);
    return inl(PCI_CFG_DATA_REGISTER);
}
static inline void pci_write32(uint32_t addr, uint32_t val)
{
    outl(PCI_CFG_ADDR_REGISTER, addr);
    outl(PCI_CFG_DATA_REGISTER, val);
}
void scan_pci(void);
uint8_t pci_find_device(uint8_t base_class, uint8_t sub_class, pci_device_t *out);
void pci_enable_bus_master(pci_device_t *dev);

#endif
