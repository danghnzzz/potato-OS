#include <stdint.h>
#include <stddef.h>
#include <kernel/interrupts.h>
#include <kernel/syscall.h>
#include <kernel/tty.h>
#include <kernel/console.h>

static syscall_handler_t syscall_handlers[MAX_NUM_SYSCALL];

static void syscall_dispatch(syscall_registers_t *regs)
{
    uint32_t num = regs->eax;
    if (num < MAX_NUM_SYSCALL && syscall_handlers[num])
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
    if (num < MAX_NUM_SYSCALL)
    {
        syscall_handlers[num] = handler;
    }
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

void init_syscall(void)
{
    console_puts("Setting up syscalls ... ");
    idt_set_gate(0x80, (uint32_t) syscall_entry, 0xef);
    register_syscall(SYS_READ, sys_read);
    register_syscall(SYS_WRITE, sys_write);
    console_puts("Done\n");
}
