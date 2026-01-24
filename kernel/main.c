#include <kernel/console.h>
#include <kernel/gdt.h>
#include <kernel/paging.h>
#include <kernel/interrupts.h>
#include <kernel/exceptions.h>
#include <kernel/syscall.h>
#include <kernel/keyboard.h>

int main()
{
    enable_console_cursor();
    console_puts("Hello, World!\n");
    console_puts("Registering global descriptor table ... ");
    init_gdt();
    console_puts("Done\n");
    console_puts("Enabling memory paging ... ");
    enable_paging();
    console_puts("Done\n");
    console_puts("Setting up CPU interrupts ... ");
    init_interrupts();
    console_puts("Done\n");
    console_puts("Setting up CPU exceptions ... ");
    init_exceptions();
    console_puts("Done\n");
    console_puts("Setting up syscalls ... ");
    init_syscall();
    console_puts("Done\n");
    console_puts("Initializing keyboard ... ");
    init_keyboard();
    console_puts("Done\n");
    for(;;);
    return 0;
}
