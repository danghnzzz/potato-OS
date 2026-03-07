#include <stdint.h>
#include <kernel/tty.h>
#include <kernel/console.h>

static volatile char tty_queue[TTY_QUEUE_SIZE];
static volatile uint32_t tty_queue_head = 0;
static volatile uint32_t tty_queue_tail = 0;

static inline uint32_t irq_save(void)
{
    uint32_t flags;
    __asm__ volatile(
        "pushf\n"
        "pop %0\n"
        "cli\n"
        : "=r"(flags)
        :
        : "memory"
    );
    return flags;
}

static inline void irq_restore(uint32_t flags)
{
    __asm__ volatile(
        "push %0\n"
        "popf\n"
        :
        : "r"(flags)
        : "memory", "cc"
    );
}

static inline int8_t queue_empty(void)
{
    return tty_queue_head == tty_queue_tail;
}

static inline void tty_queue_push_char(char c)
{
    tty_queue[tty_queue_head] = c;
    tty_queue_head = (tty_queue_head + 1) % TTY_QUEUE_SIZE;
}

static inline char tty_queue_pop_char(void)
{
    char c = tty_queue[tty_queue_tail];
    tty_queue_tail = (tty_queue_tail + 1) % TTY_QUEUE_SIZE;
    return c;
}

uint8_t init_tty(void)
{
    console_puts("Setting up TTY ... ");
    tty_queue_head = 0;
    tty_queue_tail = 0;
    console_puts("Done\n");
    return 1;
}

void tty_putc(char c)
{
    if (!c)
    {
        return;
    }
    if (c == '\r')
    {
        c = '\n';
    }
    tty_queue_push_char(c);
}

ssize_t tty_read(char *buf, uint32_t count)
{
    ssize_t read = 0;
    for (; read < count; read++)
    {
        while (queue_empty())
        {
            __asm__ volatile("hlt");
        }
        buf[read] = tty_queue_pop_char();
    }
    return read;
}

ssize_t tty_write(const char *buf, uint32_t count)
{
    ssize_t written = 0;
    for (; written < count; written++)
    {
        console_putc(buf[written]);
    }
    return written;
}
