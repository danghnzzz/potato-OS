#include <stdint.h>
#include <stddef.h>
#include <kernel/console.h>
#include <kernel/gdt.h>
#include <kernel/paging.h>
#include <kernel/interrupts.h>
#include <kernel/exceptions.h>
#include <kernel/syscall.h>
#include <kernel/tty.h>
#include <kernel/tss.h>
#include <kernel/keyboard.h>

extern uint8_t *user_stack_top;
extern void user_entry_point(void);

#define KERNEL_STACK_SIZE 8192

static uint8_t kernel_stack[KERNEL_STACK_SIZE] __attribute__((aligned(16)));
uint8_t *kernel_stack_top = kernel_stack + KERNEL_STACK_SIZE;

static void enter_user_space(uint32_t entry_point)
{
    uint32_t eflags;
    __asm__ volatile(
        "pushf\n"
        "pop %0"
        : "=r"(eflags)
        :
        : "memory"
    );
    eflags |= 0x200;
    __asm__ volatile(
        "cli\n"
        "mov eax, %[user_ss]\n"
        "mov ds, ax\n"
        "mov es, ax\n"
        "mov fs, ax\n"
        "mov gs, ax\n"
        "push %[user_ss]\n"
        "push %[user_esp]\n"
        "push %[user_eflags]\n"
        "push %[user_cs]\n"
        "push %[user_eip]\n"
        "iret\n"
        :
        : [user_ss] "r"((uint32_t) GDT_USER_DATA_SELECTOR),
          [user_esp] "r"((uint32_t) user_stack_top),
          [user_eflags] "r"(eflags),
          [user_cs] "r"((uint32_t) GDT_USER_CODE_SELECTOR),
          [user_eip] "r"(entry_point)
        : "eax", "memory"
    );
}

int main()
{
    __asm__ volatile(
        "mov esp, %0"
        :
        : "r"(kernel_stack_top)
        : "memory"
    );
    enable_console_cursor();
    console_puts("Hello, World!\n");
    console_puts("Registering global descriptor table ... ");
    init_gdt();
    console_puts("Done\n");
    console_puts("Enabling memory paging ... ");
    enable_paging();
    console_puts("Done\n");
    console_puts("Setting up CPU interrupts ... ");
    init_interrupts();
    console_puts("Done\n");
    console_puts("Setting up CPU exceptions ... ");
    init_exceptions();
    console_puts("Done\n");
    console_puts("Setting up syscalls ... ");
    init_syscall();
    console_puts("Done\n");
    console_puts("Setting up TTY ... ");
    init_tty();
    console_puts("Done\n");
    console_puts("Setting up task state segment ... ");
    init_tss((uint32_t) kernel_stack_top);
    console_puts("Done\n");
    console_puts("Initializing keyboard ... ");
    init_keyboard();
    console_puts("Done\n");
    console_puts("Entering user-space ...\n");
    enter_user_space((uint32_t) user_entry_point);
    for(;;);
    return 0;
}
