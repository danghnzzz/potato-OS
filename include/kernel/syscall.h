#ifndef SYSCALL_H
#define SYSCALL_H

#include <stddef.h>
#include <stdint.h>

#define SYSCALL_HANDLER_ENTRIES 256
#define SYS_READ 0x03
#define SYS_WRITE 0x04

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

void init_syscall(void);

#endif
