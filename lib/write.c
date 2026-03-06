#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <kernel/syscall.h>

ssize_t write(int32_t fd, const void *buf, uint32_t count)
{
    ssize_t written;
    __asm__ volatile(
        "int 0x80"
        : "=a" (written)
        : "0" (SYS_WRITE),
          "b" (fd),
          "c" ((uint32_t) buf),
          "d" (count)
        : "memory"
    );
    return written;
}
