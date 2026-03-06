#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <kernel/syscall.h>

ssize_t read(int32_t fd, void *buf, uint32_t count)
{
    ssize_t received;
    __asm__ volatile(
        "int 0x80"
        : "=a" (received)
        : "0" (SYS_READ),
          "b" (fd),
          "c" ((uint32_t) buf),
          "d" (count)
        : "memory"
    );
    return received;
}
