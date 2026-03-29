#include <stdint.h>
#include <unistd.h>
#include <kernel/syscall.h>

int32_t execve(const char *pathname, char *const argv[], char *const envp[])
{
    int32_t ret;
    __asm__ volatile(
        "int 0x80"
        : "=a"(ret)
        : "0"(SYS_EXECVE),
          "b"((uint32_t) pathname),
          "c"((uint32_t) argv),
          "d"((uint32_t) envp)
        : "memory"
    );
    return ret;
}
