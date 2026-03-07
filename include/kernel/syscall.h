#ifndef SYSCALL_H
#define SYSCALL_H

#include <stddef.h>
#include <stdint.h>

#define SYSCALL_HANDLER_ENTRIES 256
#define SYS_EXIT 0x1
#define SYS_FORK 0x2
#define SYS_READ 0x3
#define SYS_WRITE 0x4
#define SYS_OPEN 0x5
#define SYS_CLOSE 0x6

typedef struct
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
} syscall_registers_t;
typedef ssize_t (*syscall_handler_t)(syscall_registers_t *regs);

uint8_t init_syscall(void);

#endif
