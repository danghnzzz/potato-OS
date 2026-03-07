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

uint8_t init_timer()
{
    console_puts("Initializing PIT timer ... ");
    uint32_t ticks_per_int = PIT_BASE_FREQUENCY / TIMER_HZ;
    outb(PIT_CONTROL_REGISTER, 0x36);
    outb(PIT_CHANNEL_0_DATA_REGISTER, (uint8_t) ticks_per_int);
    outb(PIT_CHANNEL_0_DATA_REGISTER, (uint8_t) (ticks_per_int >> 8));
    register_irq_handler(0, timer_irq_handler);
    unmask_irq(0);
    console_puts("Done\n");
    return 1;
}

uint32_t get_current_ticks()
{
    return timer_ticks;
}
