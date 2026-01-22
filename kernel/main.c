#include <kernel/console.h>
#include <kernel/paging.h>
#include <kernel/interrupts.h>
#include <kernel/keyboard.h>

int main()
{
    enable_console_cursor();
    console_puts("Hello, World!\n");
    console_puts("Enabling memory paging...");
    enable_paging();
    console_puts("Done\n");
    console_puts("Setting up CPU interrupts...");
    init_interrupts();
    console_puts("Done\n");
    console_puts("Initializing keyboard...");
    init_keyboard();
    console_puts("Done\n");
    for(;;);
    return 0;
}
