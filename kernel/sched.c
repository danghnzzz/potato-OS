#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <kernel/console.h>
#include <kernel/paging.h>
#include <kernel/process.h>
#include <kernel/tss.h>

static task_t *task_ptr;
static task_t *current_task;
static task_t idle_task;
static uint8_t idle_task_stack[PROCESS_KERNEL_STACK_SIZE];

void set_current_task(task_t *task)
{
    current_task = task;
}

task_t *get_current_task(void)
{
    return current_task;
}

__attribute__((noreturn)) static void idle_task_entry(void)
{
    for (;;)
    {
        __asm__ volatile(
            "sti\n"
            "hlt\n"
            :
            :
            : "memory"
        );
    }
}

uint8_t init_scheduler(void)
{
    console_puts("Initializing scheduler ... ");
    task_ptr = 0;
    current_task = 0;
    console_puts("Done\n");
    return 1;
}

static task_t *next_task(void)
{
    if (!task_ptr)
    {
        return 0;
    }
    task_t *current = get_current_task();
    task_t *iter = current ? current->next : task_ptr->next;
    task_t *start = iter;
    do
    {
        if ((iter->state == TASK_READY || iter->state == TASK_RUNNING) && iter != &idle_task)
        {
            return iter;
        }
        iter = iter->next;
    }
    while (iter != start);
    return &idle_task;
}

uint8_t enqueue_task(task_t *task)
{
    if (!task || task->prev || task->next || task->state != TASK_NEW)
    {
        return 0;
    }
    if (!task_ptr)
    {
        task_ptr = task;
        task->prev = task;
        task->next = task;
        if (!get_current_task())
        {
            set_current_task(task);
            task->state = TASK_RUNNING;
            return 1;
        }
        task->state = TASK_READY;
        return 1;
    }
    task_t *head = task_ptr->next;
    task->prev = task_ptr;
    task->next = head;
    head->prev = task;
    task_ptr->next = task;
    task_ptr = task;
    if (!get_current_task())
    {
        set_current_task(task);
        task->state = TASK_RUNNING;
        return 1;
    }
    task->state = TASK_READY;
    return 1;
}

uint8_t dequeue_task(task_t *task)
{
    if (!task || !task->prev || !task->next)
    {
        return 0;
    }
    if (task->next == task)
    {
        task_ptr = 0;
        if (get_current_task() == task)
        {
            set_current_task(0);
        }
    }
    else
    {
        task->prev->next = task->next;
        task->next->prev = task->prev;
        if (task_ptr == task)
        {
            task_ptr = task->prev;
        }
        if (get_current_task() == task)
        {
            set_current_task(task->next);
        }
    }
    task->prev = 0;
    task->next = 0;
    return 1;
}

__attribute__((naked)) static void switch_to(task_t *prev, task_t *next)
{
    __asm__ volatile(
        "push ebp\n"
        "push ebx\n"
        "push esi\n"
        "push edi\n"
        "mov eax, [esp + 20]\n"
        "mov [eax + %c[kernel_esp_offset]], esp\n"
        "mov eax, [esp + 24]\n"
        "mov esp, [eax + %c[kernel_esp_offset]]\n"
        "pop edi\n"
        "pop esi\n"
        "pop ebx\n"
        "pop ebp\n"
        "ret\n"
        :
        : [kernel_esp_offset] "i"(__builtin_offsetof(task_t, kernel_esp))
        : "eax", "memory"
    );
}

static void context_switch(task_t *prev, task_t *next)
{
    if (!prev || !next || prev == next)
    {
        return;
    }
    set_current_task(next);
    next->state = TASK_RUNNING;
    tss_set_kernel_stack(next->kernel_stack_top);
    if (get_cr3() != next->cr3)
    {
        set_cr3(next->cr3);
    }
    switch_to(prev, next);
}

void block_task(task_t *task)
{
    if (!task || task == &idle_task)
    {
        return;
    }
    if (task->state != TASK_READY && task->state != TASK_RUNNING)
    {
        return;
    }
    task->state = TASK_BLOCKED;
    task_t *current = get_current_task();
    if (task != current)
    {
        return;
    }
    task_t *next = next_task();
    if (!next || next == task)
    {
        spin_forever();
    }
    context_switch(task, next);
    task->state = TASK_RUNNING;
}

void wake_task(task_t *task)
{
    if (!task || task->state != TASK_BLOCKED)
    {
        return;
    }
    task->state = TASK_READY;
}

void schedule(void)
{
    task_t *current = get_current_task();
    if (!current)
    {
        return;
    }
    task_t *next = next_task();
    if (!next || next == current)
    {
        return;
    }
    current->state = TASK_READY;
    context_switch(current, next);
    current->state = TASK_RUNNING;
}

uint8_t init_idle_task(void)
{
    console_puts("Initializing idle task ... ");
    idle_task.pid = 0;
    idle_task.parent = 0;
    idle_task.state = TASK_NEW;
    idle_task.cr3 = get_cr3();
    idle_task.kernel_stack_base = (uintptr_t) idle_task_stack;
    idle_task.kernel_stack_top = idle_task.kernel_stack_base + PROCESS_KERNEL_STACK_SIZE;
    uint32_t *stack = (uint32_t *) idle_task.kernel_stack_top;
    *--stack = (uint32_t) idle_task_entry;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    idle_task.kernel_esp = (uintptr_t) stack;
    idle_task.prev = &idle_task;
    idle_task.next = &idle_task;
    idle_task.state = TASK_READY;
    task_ptr = &idle_task;
    console_puts("Done\n");
    return 1;
}

__attribute__((noreturn, naked)) void enter_user_task(
    uint32_t user_eip,
    uint32_t user_cs,
    uint32_t user_eflags,
    uint32_t user_esp,
    uint32_t user_ss
)
{
    __asm__ volatile(
        "cli\n"
        "mov eax, [esp + 20]\n"
        "mov ds, ax\n"
        "mov es, ax\n"
        "mov fs, ax\n"
        "mov gs, ax\n"
        "add esp, 4\n"
        "iret\n"
        :
        :
        : "eax", "memory"
    );
}
