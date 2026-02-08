#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include <kernel/io.h>

#define PCI_CFG_ADDR_REGISTER 0xcf8
#define PCI_CFG_DATA_REGISTER 0xcfc

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

#endif
