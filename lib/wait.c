#include <stdint.h>
#include <unistd.h>
#include <kernel/syscall.h>

int32_t waitpid(int32_t pid, int32_t *status, int32_t options)
{
    int32_t ret;
    __asm__ volatile(
        "int 0x80"
        : "=a"(ret)
        : "0"(SYS_WAITPID),
          "b"(pid),
          "c"(status),
          "d"(options)
        : "memory"
    );
    return ret;
}
