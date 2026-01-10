#include <kernel/console.h>
#include <kernel/paging.h>

int main()
{
    enable_console_cursor();
    console_puts("Hello, World!\n");
    console_puts("Enabling memory paging...");
    enable_paging();
    console_puts("Done");
    return 0;
}
