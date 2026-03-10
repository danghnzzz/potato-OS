#include <stdint.h>
#include <kernel/io.h>
#include <kernel/interrupts.h>
#include <kernel/console.h>
#include <kernel/gdt.h>

static idt_entry idt[IDT_ENTRIES];
static irq_handler_t irq_handlers[16];

static void pic_remap(void)
{
    uint8_t a1 = inb(MASTER_PIC_DATA_REGISTER);
    uint8_t a2 = inb(SLAVE_PIC_DATA_REGISTER);
    outb(MASTER_PIC_COMMAND_REGISTER, 0x11);
    outb(SLAVE_PIC_COMMAND_REGISTER, 0x11);
    outb(MASTER_PIC_DATA_REGISTER, 0x20);
    outb(SLAVE_PIC_DATA_REGISTER, 0x28);
    outb(MASTER_PIC_DATA_REGISTER, 0x4);
    outb(SLAVE_PIC_DATA_REGISTER, 0x2);
    outb(MASTER_PIC_DATA_REGISTER, 0x1);
    outb(SLAVE_PIC_DATA_REGISTER, 0x1);
    outb(MASTER_PIC_DATA_REGISTER, a1);
    outb(SLAVE_PIC_DATA_REGISTER, a2);
}

static void init_idt(void)
{
    for (uint16_t i = 0; i < IDT_ENTRIES; i++)
    {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].reserved = 0;
        idt[i].type_attr = 0;
        idt[i].offset_high = 0;
    }
}

void mask_irq(uint8_t irq)
{
    uint16_t port = (irq < 8) ? MASTER_PIC_DATA_REGISTER : SLAVE_PIC_DATA_REGISTER;
    uint8_t value = inb(port) | (1 << (irq & 0x7));
    outb(port, value);
}

void unmask_irq(uint8_t irq)
{
    uint16_t port = (irq < 8) ? MASTER_PIC_DATA_REGISTER : SLAVE_PIC_DATA_REGISTER;
    uint8_t value = inb(port) & ~(1 << (irq & 0x7));
    outb(port, value);
}

static void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8)
    {
        outb(SLAVE_PIC_COMMAND_REGISTER, 0x20);
    }
    outb(MASTER_PIC_COMMAND_REGISTER, 0x20);
}

void interrupts_dispatch(uint8_t irq)
{
    if (irq < 16 && irq_handlers[irq])
    {
        irq_handlers[irq]();
    }
    pic_send_eoi(irq);
}

DECLARE_IRQ(0)
DECLARE_IRQ(1)
DECLARE_IRQ(2)
DECLARE_IRQ(3)
DECLARE_IRQ(4)
DECLARE_IRQ(5)
DECLARE_IRQ(6)
DECLARE_IRQ(7)
DECLARE_IRQ(8)
DECLARE_IRQ(9)
DECLARE_IRQ(10)
DECLARE_IRQ(11)
DECLARE_IRQ(12)
DECLARE_IRQ(13)
DECLARE_IRQ(14)
DECLARE_IRQ(15)

void idt_set_gate(uint8_t idx, uint32_t handler, uint8_t interrupts_gate_flags)
{
    idt[idx].offset_low = (uint16_t) handler;
    idt[idx].selector = GDT_KERNEL_CODE_SELECTOR;
    idt[idx].reserved = 0;
    idt[idx].type_attr = interrupts_gate_flags;
    idt[idx].offset_high = (uint16_t) (handler >> 16);
}

static void install_irq_gates(void)
{
    idt_set_gate(32, (uint32_t) irq0, 0x8e);
    idt_set_gate(33, (uint32_t) irq1, 0x8e);
    idt_set_gate(34, (uint32_t) irq2, 0x8e);
    idt_set_gate(35, (uint32_t) irq3, 0x8e);
    idt_set_gate(36, (uint32_t) irq4, 0x8e);
    idt_set_gate(37, (uint32_t) irq5, 0x8e);
    idt_set_gate(38, (uint32_t) irq6, 0x8e);
    idt_set_gate(39, (uint32_t) irq7, 0x8e);
    idt_set_gate(40, (uint32_t) irq8, 0x8e);
    idt_set_gate(41, (uint32_t) irq9, 0x8e);
    idt_set_gate(42, (uint32_t) irq10, 0x8e);
    idt_set_gate(43, (uint32_t) irq11, 0x8e);
    idt_set_gate(44, (uint32_t) irq12, 0x8e);
    idt_set_gate(45, (uint32_t) irq13, 0x8e);
    idt_set_gate(46, (uint32_t) irq14, 0x8e);
    idt_set_gate(47, (uint32_t) irq15, 0x8e);
}

static void lock_irqs(void)
{
    mask_irq(0);
    mask_irq(1);
    mask_irq(2);
    mask_irq(3);
    mask_irq(4);
    mask_irq(5);
    mask_irq(6);
    mask_irq(7);
    mask_irq(8);
    mask_irq(9);
    mask_irq(10);
    mask_irq(11);
    mask_irq(12);
    mask_irq(13);
    mask_irq(14);
    mask_irq(15);
}

static void lidt(void *base, uint16_t size)
{
    idt_descriptor_t desc = {
        .limit = size - 1,
        .base = (uint32_t) base
    };
    __asm__ volatile(
        "lidt [%0]"
        :
        : "r"(&desc)
        :
    );
}

static void enable_interrupts(void)
{
    __asm__ volatile("sti");
}

uint8_t init_interrupts(void)
{
    console_puts("Setting up CPU interrupts ... ");
    pic_remap();
    init_idt();
    install_irq_gates();
    lock_irqs();
    lidt(idt, sizeof(idt));
    enable_interrupts();
    console_puts("Done\n");
    return 1;
}

void register_irq_handler(uint8_t irq, irq_handler_t handler)
{
    if (irq < 16)
    {
        irq_handlers[irq] = handler;
    }
}
