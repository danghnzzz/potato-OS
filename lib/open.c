#include <stdint.h>
#include <unistd.h>
#include <kernel/syscall.h>

int32_t open(const char *pathname, int32_t flags)
{
    int ret;
    __asm__ volatile (
        "int 0x80"
        : "=a" (ret)
        : "0" (SYS_OPEN),
          "b" (pathname),
          "c" (flags)
        : "memory"
    );
    return ret;
}