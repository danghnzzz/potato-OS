#include <stdint.h>
#include <stddef.h>
#include <kernel/console.h>
#include <kernel/gdt.h>
#include <kernel/paging.h>
#include <kernel/pci.h>
#include <kernel/interrupts.h>
#include <kernel/exceptions.h>
#include <kernel/syscall.h>
#include <kernel/tty.h>
#include <kernel/tss.h>
#include <kernel/timer.h>
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
        "pop %0\n"
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
    init_gdt();
    enable_paging();
    scan_pci();
    init_interrupts();
    init_exceptions();
    init_syscall();
    init_tty();
    init_tss((uint32_t) kernel_stack_top);
    init_timer();
    init_keyboard();
    console_puts("Entering user-space ...\n");
    enter_user_space((uint32_t) user_entry_point);
    for(;;);
    return 0;
}
