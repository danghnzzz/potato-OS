#include <stdint.h>
#include <unistd.h>
#include <kernel/syscall.h>

int32_t fork(void)
{
    int32_t ret;
    __asm__ volatile(
        "int 0x80"
        : "=a"(ret)
        : "0"(SYS_FORK)
        : "memory"
    );
    return ret;
}
