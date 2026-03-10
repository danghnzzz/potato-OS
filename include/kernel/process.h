#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <kernel/fs.h>

#define PROCESS_FDS 32

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
    uint32_t eip;
    uint32_t eflags;
    uint32_t cr3;
} task_context_t;
typedef struct task_struct_t
{
    uint32_t pid;
    uint32_t ppid;
    task_context_t context;
    uintptr_t kernel_stack_base;
    uintptr_t kernel_stack_top;
    file_t *file[PROCESS_FDS];
    struct task_struct_t *prev;
    struct task_struct_t *next;
} task_t;

#endif
