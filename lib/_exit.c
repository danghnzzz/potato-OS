#include <unistd.h>
#include <kernel/syscall.h>

__attribute__((noreturn)) void _exit(int status_code)
{
    for (;;) 
    {
        __asm__ volatile(
            "int 0x80"
            :
            : "a" (SYS_EXIT),
              "b" (status_code)
            : "memory"
        );
    }
}
