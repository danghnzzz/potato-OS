#include <stdint.h>
#include <io.h>
#include <kernel/console.h>
#include <kernel/interrupts.h>
#include <kernel/keyboard.h>

static const char scancode_map[128] =
{
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'
};

static void keyboard_irq_handler(void)
{
    uint8_t scancode = inb(0x60);
    if (!(scancode & 0x80) && scancode < sizeof(scancode_map))
    {
        char c = scancode_map[scancode];
        if (c)
        {
            console_putc(c);
        }
    }
}

void init_keyboard(void)
{
    register_irq_handler(1, keyboard_irq_handler);
    unmask_irq(1);
}
