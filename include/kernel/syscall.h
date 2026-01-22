#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

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
typedef uint32_t (*syscall_handler_t)(syscall_registers_t *regs);

#define MAX_NUM_SYSCALL 256

void init_syscall(void);
void register_syscall(uint8_t num, syscall_handler_t handler);

#endif
