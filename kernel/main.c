#include <stdint.h>
#include <string.h>
#include <kernel/console.h>
#include <kernel/gdt.h>
#include <kernel/pci.h>
#include <kernel/interrupts.h>
#include <kernel/exceptions.h>
#include <kernel/syscall.h>
#include <kernel/tty.h>
#include <kernel/tss.h>
#include <kernel/timer.h>
#include <kernel/disk.h>
#include <kernel/fs.h>
#include <kernel/keyboard.h>
#include <kernel/process.h>
#include <kernel/paging.h>
#include <kernel/heap.h>

extern uint8_t kernel_stack_top[];
extern void user_entry_point(void);

#define PROC1_VIRT_BASE 0x00400000

static void enter_proc1(uint32_t entry_point)
{
    task_t *proc1 = kmalloc(sizeof(task_t));
    set_current_task(proc1);
    mm_t *mm1 = kmalloc(sizeof(mm_t));
    uint32_t eflags;
    __asm__ volatile(
        "pushf\n"
        "pop %0\n"
        : "=r"(eflags)
        :
        : "memory"
    );
    eflags |= 0x200;
    proc1->pid = 1;
    proc1->parent = 0;
    proc1->kernel_stack_base = (uintptr_t) kernel_stack_top - 8192;
    proc1->kernel_stack_top = (uintptr_t) kernel_stack_top;
    if (!create_task_mm(mm1))
    {
        console_puts("Failed to create proc1 mm\n");
        for (;;);
    }
    proc1->context.eip = entry_point;
    proc1->context.esp = (uint32_t) (PROC1_VIRT_BASE + PAGE_SIZE);
    proc1->context.ebp = (uint32_t) (PROC1_VIRT_BASE + PAGE_SIZE);
    proc1->context.eflags = eflags;
    proc1->context.cr3 = get_pd_lma((uintptr_t) mm1->pgd);
    proc1->mm = mm1;
    proc1->prev = 0;
    proc1->next = 0;
    set_cr3(proc1->context.cr3);
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
          [user_esp] "r"(proc1->context.esp),
          [user_eflags] "r"(proc1->context.eflags),
          [user_cs] "r"((uint32_t) GDT_USER_CODE_SELECTOR),
          [user_eip] "r"(proc1->context.eip)
        : "eax", "memory"
    );
}

int main()
{
    enable_console_cursor();
    console_puts("Hello, World!\n");
    init_gdt();
    scan_pci();
    init_interrupts();
    init_exceptions();
    init_syscall();
    init_tty();
    init_tss((uint32_t) kernel_stack_top);
    init_timer();
    uint8_t disk_is_ready = init_disk();
    if (disk_is_ready)
    {
        init_fs();
    }
    init_keyboard();
    console_puts("Spawning proc1 ...\n");
    enter_proc1((uint32_t) user_entry_point);
    for (;;);
    return 0;
}
