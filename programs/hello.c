#include <unistd.h>

void _start(void)
{
    const char msg[] = "|-| [- |_ |_ ()\n";
    write(1, msg, sizeof(msg) - 1);
    _exit(0);
}
