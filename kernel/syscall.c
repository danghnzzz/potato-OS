#include <stdint.h>
#include <stddef.h>
#include <kernel/elf.h>
#include <kernel/fs.h>
#include <kernel/heap.h>
#include <kernel/interrupts.h>
#include <kernel/memory.h>
#include <kernel/paging.h>
#include <kernel/process.h>
#include <kernel/syscall.h>
#include <kernel/tty.h>
#include <kernel/console.h>

static syscall_handler_t syscall_handlers[SYSCALL_HANDLER_ENTRIES];

static void syscall_dispatch(syscall_registers_t *regs)
{
    uint32_t num = regs->eax;
    if (num < SYSCALL_HANDLER_ENTRIES && syscall_handlers[num])
    {
        regs->eax = (uint32_t) syscall_handlers[num](regs);
        return;
    }
    regs->eax = (uint32_t) -1;
}

__attribute__((naked)) static void syscall_entry(void)
{
    __asm__ volatile(
        "pusha\n"
        "mov eax, esp\n"
        "push eax\n"
        "call syscall_dispatch\n"
        "add esp, 4\n"
        "popa\n"
        "iret\n"
        :
        :
        : "memory"
    );
}

static void register_syscall(uint8_t num, syscall_handler_t handler)
{
    if (num < SYSCALL_HANDLER_ENTRIES)
    {
        syscall_handlers[num] = handler;
    }
}

static ssize_t sys_exit(syscall_registers_t *regs)
{
    exit_task((int32_t) regs->ebx);
}

static ssize_t sys_read(syscall_registers_t *regs)
{
    uint32_t fd  = regs->ebx;
    char *buf = (char *) regs->ecx;
    uint32_t count = regs->edx;
    if (fd != 0)
    {
        return (ssize_t) -1;
    }
    return tty_read(buf, count);
}

static ssize_t sys_write(syscall_registers_t *regs)
{
    uint32_t fd  = regs->ebx;
    const char *buf = (const char *) regs->ecx;
    uint32_t count = regs->edx;
    if ((fd != 1 && fd != 2) || !buf)
    {
        return (ssize_t) -1;
    }
    return tty_write(buf, count);
}

static ssize_t sys_execve(syscall_registers_t *regs)
{
    const char *exec_path = (const char *) regs->ebx;
    file_t *exec_file = (file_t *) kmalloc(sizeof(*exec_file));
    if (!exec_file)
    {
        return (ssize_t) -1;
    }
    if (!vfs_open(exec_path, exec_file))
    {
        kfree(exec_file);
        return (ssize_t) -1;
    }
    mm_t *new_mm = (mm_t *) kmalloc(sizeof(*new_mm));
    if (!new_mm)
    {
        vfs_close(exec_file);
        kfree(exec_file);
        return (ssize_t) -1;
    }
    if (!create_task_mm(new_mm))
    {
        kfree(new_mm);
        vfs_close(exec_file);
        kfree(exec_file);
        return (ssize_t) -1;
    }
    task_t *task = get_current_task();
    mm_t *old_mm = task->mm;
    uint32_t entry_point;
    switch_mm(task, new_mm);
    if (!vm_brk_flags(USER_STACK_BASE, PAGE_SIZE) || !elf32_load_exec(exec_file, &entry_point))
    {
        switch_mm(task, old_mm);
        release_mm(new_mm);
        return (ssize_t) -1;
    }
    release_mm(old_mm);
    regs->edi = regs->esi = regs->ebp = 0;
    regs->ebx = regs->edx = regs->ecx = 0;
    regs->user_eip = entry_point;
    regs->user_eflags = 0x202;
    regs->user_esp = USER_STACK_TOP;
    return 0;
}

uint8_t init_syscall(void)
{
    console_puts("Setting up syscalls ... ");
    idt_set_gate(0x80, (uint32_t) syscall_entry, 0xef);
    register_syscall(SYS_EXIT, sys_exit);
    register_syscall(SYS_READ, sys_read);
    register_syscall(SYS_WRITE, sys_write);
    register_syscall(SYS_EXECVE, sys_execve);
    console_puts("Done\n");
    return 1;
}
