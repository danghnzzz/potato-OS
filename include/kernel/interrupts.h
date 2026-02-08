#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

#define MASTER_PIC_COMMAND_REGISTER 0x20
#define MASTER_PIC_DATA_REGISTER 0x21
#define SLAVE_PIC_COMMAND_REGISTER 0xa0
#define SLAVE_PIC_DATA_REGISTER 0xa1
#define IDT_ENTRIES 256
#define DECLARE_IRQ(n) \
    __attribute__((naked)) static void irq##n(void) \
    { \
        __asm__ volatile( \
            "pusha\n" \
            "push " #n "\n" \
            "call interrupts_dispatch\n" \
            "add esp, 4\n" \
            "popa\n" \
            "iret\n" \
            : \
            : \
            : \
        ); \
    }

typedef struct __attribute__((packed))
{
    uint16_t limit;
    uint32_t base;
} idt_descriptor;
typedef struct __attribute__((packed))
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t reserved;
    uint8_t type_attr;
    uint16_t offset_high;
} idt_entry;
typedef void (*irq_handler_t)(void);

void mask_irq(uint8_t irq);
void unmask_irq(uint8_t irq);
void idt_set_gate(uint8_t idx, uint32_t handler, uint8_t interrupts_gate_flags);
void init_interrupts(void);
void register_irq_handler(uint8_t irq, irq_handler_t handler);

#endif
