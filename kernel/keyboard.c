#include <stdint.h>
#include <kernel/io.h>
#include <kernel/interrupts.h>
#include <kernel/tty.h>
#include <kernel/keyboard.h>
#include <kernel/console.h>

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
    uint8_t scancode = inb(KEYBOARD_CONTROLLER_DATA_REGISTER);
    if (!(scancode & 0x80) && scancode < sizeof(scancode_map))
    {
        char c = scancode_map[scancode];
        tty_putc(c);
    }
}

void init_keyboard(void)
{
    console_puts("Initializing keyboard ... ");
    register_irq_handler(1, keyboard_irq_handler);
    unmask_irq(1);
    console_puts("Done\n");
}
