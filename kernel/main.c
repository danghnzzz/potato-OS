#include <kernel/console.h>

int main()
{
    enable_console_cursor();
    console_puts("Hello, World!");
    return 0;
}
