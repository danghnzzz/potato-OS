#include <stdint.h>
#include <kernel/gdt.h>
#include <kernel/tss.h>
#include <kernel/console.h>

static tss_entry_t tss __attribute__((aligned(16)));

static void gdt_set_tss_entry()
{
    gdt_set_entry(5, (uint32_t) &tss, sizeof(tss_entry_t) - 1, 0x89, 0x0);
}

static void init_tss_entry()
{
    tss.link = 0x0;
    tss.reserved0 = 0x0;
    tss.esp0 = 0x0;
    tss.ss0 = 0x0;
    tss.reserved1 = 0x0;
    tss.esp1 = 0x0;
    tss.ss1 = 0x0;
    tss.reserved2 = 0x0;
    tss.esp2 = 0x0;
    tss.ss2 = 0x0;
    tss.reserved3 = 0x0;
    tss.cr3 = 0x0;
    tss.eip = 0x0;
    tss.eflags = 0x0;
    tss.eax = 0x0;
    tss.ecx = 0x0;
    tss.edx = 0x0;
    tss.ebx = 0x0;
    tss.esp = 0x0;
    tss.ebp = 0x0;
    tss.esi = 0x0;
    tss.edi = 0x0;
    tss.es = 0x0;
    tss.reserved4 = 0x0;
    tss.cs = 0x0;
    tss.reserved5 = 0x0;
    tss.ss = 0x0;
    tss.reserved6 = 0x0;
    tss.ds = 0x0;
    tss.reserved7 = 0x0;
    tss.fs = 0x0;
    tss.reserved8 = 0x0;
    tss.gs = 0x0;
    tss.reserved9 = 0x0;
    tss.ldtr = 0x0;
    tss.reserved10 = 0x0;
    tss.reserved11 = 0x0;
    tss.iopb = 0x0;
}

static void ltss(void)
{
    __asm__ volatile(
        "mov ax, %0\n"
        "ltr ax\n"
        :
        : "i"(GDT_TSS_SELECTOR)
        : "ax"
    );
}

void init_tss(uint32_t stack)
{
    console_puts("Setting up task state segment ... ");
    init_tss_entry();
    tss.ss0 = GDT_KERNEL_DATA_SELECTOR;
    tss.esp0 = stack;
    tss.iopb = sizeof(tss_entry_t);
    gdt_set_tss_entry();
    ltss();
    console_puts("Done\n");
}
