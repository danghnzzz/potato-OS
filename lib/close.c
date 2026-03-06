#include <stdint.h>
#include <unistd.h>
#include <kernel/syscall.h>

int32_t close(int32_t fd)
{
    int ret;
    __asm__ volatile (
        "int 0x80"
        : "=a"(ret)
        : "0"(SYS_CLOSE),
          "b"(fd)
        : "memory"
    );
    return ret;
}