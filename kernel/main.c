#include <stdint.h>
#include <string.h>
#include <kernel/console.h>
#include <kernel/gdt.h>
#include <kernel/paging.h>
#include <kernel/memory.h>
#include <kernel/heap.h>
#include <kernel/pci.h>
#include <kernel/interrupts.h>
#include <kernel/exceptions.h>
#include <kernel/syscall.h>
#include <kernel/tty.h>
#include <kernel/tss.h>
#include <kernel/timer.h>
#include <kernel/disk.h>
#include <kernel/fs.h>
#include <kernel/elf.h>
#include <kernel/keyboard.h>
#include <kernel/process.h>
#include <kernel/fault.h>

extern uint8_t kernel_stack_top[];

#define PROC1_EXEC_PATH "/bin/potatoshell"

__attribute__((noreturn)) static void spin_forever(void)
{
    for (;;);
}

static void print_proc1_exec_info(file_t *exec)
{
    console_puts("  executable file: ");
    console_puts(PROC1_EXEC_PATH);
    console_putc('\n');
    console_puts("    time: ");
    console_put_hex32(exec->f_inode->i_time, 1);
    console_putc('\n');
    console_puts("    mode: ");
    console_put_hex16(exec->f_inode->i_mode, 1);
    console_putc('\n');
    console_puts("    size: ");
    console_put_hex32(exec->f_inode->i_size, 1);
    console_putc('\n');
    console_puts("    uid: ");
    console_put_hex16(exec->f_inode->i_uid, 1);
    console_putc('\n');
    console_puts("    gid: ");
    console_put_hex8(exec->f_inode->i_gid, 1);
    console_putc('\n');
    console_puts("    links: ");
    console_put_hex8(exec->f_inode->i_nlinks, 1);
    console_putc('\n');
}

static void enter_proc1(void)
{
    uint32_t entry_point;
    console_puts("Spawning proc1 ...\n");
    task_t *proc1 = (task_t *) kmalloc(sizeof(task_t));
    if (!proc1)
    {
        console_puts("Failed: can not allocate proc1 task\n");
        spin_forever();
    }
    memset(proc1, 0, sizeof(*proc1));
    proc1->pid = 1;
    proc1->parent = 0;
    uint8_t *stack = kmalloc(PROCESS_KERNEL_STACK_SIZE);
    if (!stack)
    {
        console_puts("Failed: can not allocate proc1 kernel stack\n");
        spin_forever();
    }
    proc1->kernel_stack_base = (uintptr_t) stack;
    proc1->kernel_stack_top = (uintptr_t) (stack + PROCESS_KERNEL_STACK_SIZE);
    tss_set_kernel_stack((uint32_t) proc1->kernel_stack_top);
    mm_t *mm1 = kmalloc(sizeof(mm_t));
    if (!mm1)
    {
        console_puts("Failed: can not allocate proc1 mm\n");
        spin_forever();
    }
    if (!create_task_mm(mm1))
    {
        console_puts("Failed: can not create proc1 mm\n");
        spin_forever();
    }
    uint32_t eflags;
    __asm__ volatile(
        "pushf\n"
        "pop %0\n"
        : "=r"(eflags)
        :
        : "memory"
    );
    eflags |= 0x200;
    proc1->context.eflags = eflags;
    proc1->context.cr3 = get_pd_lma((uintptr_t) mm1->pgd);
    proc1->mm = mm1;
    set_current_task(proc1);
    if (!vm_brk_flags(USER_STACK_BASE, PAGE_SIZE))
    {
        console_puts("Failed: can not reserve user stack\n");
        spin_forever();
    }
    proc1->context.esp = USER_STACK_TOP;
    proc1->context.ebp = USER_STACK_TOP;
    file_t *proc1_exec = (file_t *) kmalloc(sizeof(file_t));
    if (!proc1_exec || !vfs_open(PROC1_EXEC_PATH, proc1_exec))
    {
        console_puts("Failed: can not get executable file\n");
        spin_forever();
    }
    print_proc1_exec_info(proc1_exec);
    if (!elf32_load_exec(proc1_exec, &entry_point))
    {
        console_puts("Failed: can not load executable image\n");
        spin_forever();
    }
    proc1->context.eip = entry_point;
    console_puts("Done\n");
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
    uint8_t disk_is_ready = 0;
    uint8_t fs_is_ready = 0;
    uint8_t fault_handler_is_ready = 0;
    enable_console_cursor();
    console_puts("Hello, World!\n");
    init_gdt();
    init_physical_memory_allocator();
    init_heap();
    scan_pci();
    init_interrupts();
    init_exceptions();
    init_syscall();
    init_tty();
    init_tss((uint32_t) kernel_stack_top);
    init_timer();
    disk_is_ready = init_disk();
    if (disk_is_ready)
    {
        fs_is_ready = init_fs();
    }
    fault_handler_is_ready = init_fault();
    init_keyboard();
    if (fs_is_ready && fault_handler_is_ready)
    {
        enter_proc1();
    }
    spin_forever();
    return 0;
}
