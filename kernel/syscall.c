#include <stdint.h>
#include <kernel/console.h>
#include <kernel/interrupts.h>
#include <kernel/syscall.h>

static syscall_handler_t syscall_handlers[MAX_NUM_SYSCALL];

static void syscall_dispatch(syscall_registers_t *regs)
{
    uint32_t num = regs->eax;
    if (num < MAX_NUM_SYSCALL && syscall_handlers[num])
    {
        regs->eax = syscall_handlers[num](regs);
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

static uint32_t sys_write(syscall_registers_t *regs)
{
    const char *buf = (const char *) regs->ecx;
    uint32_t len = regs->edx;
    if (!buf)
    {
        return (uint32_t) -1;
    }
    uint32_t written = 0;
    for (; written < len; written++)
    {
        console_putc(buf[written]);
    }
    return written;
}

void init_syscall(void)
{
    idt_set_gate(0x80, (uint32_t) syscall_entry, 0xef);
    register_syscall(SYS_WRITE, sys_write);
}
