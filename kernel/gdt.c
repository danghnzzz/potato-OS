#include <stdint.h>
#include <kernel/gdt.h>
#include <kernel/console.h>

static gdt_entry_t gdt[GDT_ENTRIES] __attribute__((aligned(8)));

void gdt_set_entry(uint8_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity)
{
    gdt[idx].limit_low = (uint16_t) limit;
    gdt[idx].base_low = (uint16_t) base;
    gdt[idx].base_middle = (uint8_t) (base >> 16);
    gdt[idx].access = access;
    gdt[idx].granularity = ((limit >> 16) & 0xf) | (granularity & 0xf0);
    gdt[idx].base_high = (uint8_t) (base >> 24);
}

static void lgdt(void *base, uint16_t size)
{
    gdt_descriptor_t desc = {
        .limit = size - 1,
        .base = (uint32_t) base
    };
    __asm__ volatile(
        "lgdt [%0]"
        :
        : "r"(&desc)
        : "memory"
    );
}

static void reload_segment_registers(void)
{
    __asm__ volatile(
        "mov ax, %0\n"
        "mov ds, ax\n"
        "mov es, ax\n"
        "mov fs, ax\n"
        "mov gs, ax\n"
        "mov ss, ax\n"
        "jmp %1:.done_gdt\n"
        ".done_gdt:\n"
        :
        : "i"(GDT_KERNEL_DATA_SELECTOR), "i"(GDT_KERNEL_CODE_SELECTOR)
        : "ax", "memory"
    );
}

uint8_t init_gdt(void)
{
    console_puts("Registering global descriptor table ... ");
    gdt_set_entry(0, 0, 0, 0, 0);
    gdt_set_entry(1, 0x0, 0xfffff, 0x9a, 0xcf);
    gdt_set_entry(2, 0x0, 0xfffff, 0x92, 0xcf);
    gdt_set_entry(3, 0x0, 0xfffff, 0xfa, 0xcf);
    gdt_set_entry(4, 0x0, 0xfffff, 0xf2, 0xcf);
    lgdt(gdt, sizeof(gdt));
    reload_segment_registers();
    console_puts("Done\n");
    return 1;
}
