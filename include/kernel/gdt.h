#ifndef GDT_H
#define GDT_H

#include <stdint.h>

typedef struct __attribute__((packed))
{
    uint16_t limit;
    uint32_t base;
} gdt_descriptor;
typedef struct __attribute__((packed))
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} gdt_entry_t;

#define GDT_ENTRIES 6
#define GDT_KERNEL_CODE_SELECTOR 0x08
#define GDT_KERNEL_DATA_SELECTOR 0x10
#define GDT_USER_CODE_SELECTOR (0x18 | 0x3)
#define GDT_USER_DATA_SELECTOR (0x20 | 0x3)
#define GDT_TSS_SELECTOR 0x28

void gdt_set_entry(uint8_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity);
void init_gdt(void);

#endif
