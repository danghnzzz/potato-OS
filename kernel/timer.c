#include <stdint.h>
#include <kernel/io.h>
#include <kernel/interrupts.h>
#include <kernel/timer.h>
#include <kernel/console.h>

static volatile uint32_t timer_ticks = 0;

static void timer_irq_handler(void)
{
    timer_ticks++;
}

void init_timer()
{
    uint32_t ticks_per_int = PIT_BASE_FREQUENCY / TIMER_HZ;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(ticks_per_int & 0xff));
    outb(0x40, (uint8_t)((ticks_per_int >> 8) & 0xff));
    register_irq_handler(0, timer_irq_handler);
    unmask_irq(0);
}
