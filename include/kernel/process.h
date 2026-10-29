#ifndef PROCESS_H
#define PROCESS_H

#include <stddef.h>
#include <stdint.h>
#include <kernel/memory.h>
#include <kernel/fs.h>

#define PROCESS_FDS 32
#define PROCESS_KERNEL_STACK_SIZE 8192
#define SCHEDULER_TICK_INTERVAL 5
#define TASK_NEW 0
#define TASK_READY 1
#define TASK_RUNNING 2
#define TASK_BLOCKED 3
#define TASK_ZOMBIE 4

typedef struct task_struct_t
{
    uint32_t pid;
    struct task_struct_t *parent;
    uint8_t state;
    int32_t exit_code;
    mm_t *mm;
    uintptr_t kernel_stack_base;
    uintptr_t kernel_stack_top;
    uintptr_t kernel_esp;
    uintptr_t cr3;
    file_t *file[PROCESS_FDS];
    struct task_struct_t *prev;
    struct task_struct_t *next;
} task_t;

uint32_t alloc_pid(void);
uint8_t create_task_mm(mm_t *mm);
void release_mm(mm_t *mm);
void switch_mm(task_t *task, mm_t *mm);
task_t *create_task(task_t *parent);
void destroy_task(task_t *task);
void set_current_task(task_t *task);
task_t *get_current_task(void);
uint8_t init_scheduler(void);
uint8_t enqueue_task(task_t *task);
uint8_t dequeue_task(task_t *task);
void block_task(task_t *task);
void wake_task(task_t *task);
void schedule(void);
__attribute__((noreturn)) void exit_task(int32_t exit_code);
uint8_t init_idle_task(void);
__attribute__((noreturn, naked)) void enter_user_task(
    uint32_t user_eip,
    uint32_t user_cs,
    uint32_t user_eflags,
    uint32_t user_esp,
    uint32_t user_ss
);
uint8_t prepare_user_task_stack(task_t *task, uint32_t entry_point, uint32_t user_eflags, uint32_t user_esp);
__attribute__((noreturn)) void spin_forever(void);

#endif
