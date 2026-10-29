#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <kernel/gdt.h>
#include <kernel/heap.h>
#include <kernel/process.h>
#include <kernel/memory.h>
#include <kernel/paging.h>

static uint32_t pid_count = 0;

uint32_t alloc_pid(void)
{
    return ++pid_count;
}

static void copy_kernel_mappings(uintptr_t *dst_pgd, uintptr_t *src_pgd)
{
    for (uint32_t i = 0; i < PAGE_DIRECTORY_ENTRIES; i++)
    {
        uintptr_t pde = src_pgd[i];
        if ((pde & PAGE_FLAG_PRESENT) && !(pde & PAGE_FLAG_USER))
        {
            dst_pgd[i] = pde;
        }
    }
}

uint8_t create_task_mm(mm_t *mm)
{
    uintptr_t *pgd = alloc_pd();
    if (!pgd)
    {
        return 0;
    }
    uintptr_t *current_pgd = (uintptr_t *) get_pd_vma(get_cr3());
    copy_kernel_mappings(pgd, current_pgd);
    mm->pgd = pgd;
    mm->mmap = 0;
    return 1;
}

static uint8_t vma_file_seen(vm_area_t *head, vm_area_t *stop, file_t *file)
{
    for (vm_area_t *iter = head; iter != stop; iter = iter->vm_next)
    {
        if (iter->vm_file == file)
        {
            return 1;
        }
    }
    return 0;
}

void release_task_mm(task_t *task)
{
    mm_t *mm = task->mm;
    if (!mm)
    {
        return;
    }
    for (vm_area_t *vma = mm->mmap; vma; vma = vma->vm_next)
    {
        file_t *file = vma->vm_file;
        if (!file || vma_file_seen(mm->mmap, vma, file))
        {
            continue;
        }
        vfs_close(file);
        kfree(file);
    }
    vm_area_t *vma = mm->mmap;
    while (vma)
    {
        vm_area_t *next = vma->vm_next;
        kfree(vma);
        vma = next;
    }
    if (mm->pgd)
    {
        for (uint32_t pde_index = 0; pde_index < PAGE_DIRECTORY_ENTRIES; pde_index++)
        {
            uintptr_t pde = mm->pgd[pde_index];
            if (!(pde & PAGE_FLAG_PRESENT) || !(pde & PAGE_FLAG_USER))
            {
                continue;
            }
            uintptr_t *page_table = (uintptr_t *) get_pd_vma(pde);
            for (uint32_t pte_index = 0; pte_index < PAGE_TABLE_ENTRIES; pte_index++)
            {
                uintptr_t pte = page_table[pte_index];
                if (pte & PAGE_FLAG_PRESENT)
                {
                    free_phys_frame(pte & PAGE_ADDRESS_MASK);
                }
            }

            free_pt(page_table);
            mm->pgd[pde_index] = 0;
        }

        free_pd(mm->pgd);
    }
    task->mm = 0;
    kfree(mm);
}

task_t *create_task(task_t *parent)
{
    task_t *task = (task_t *) kmalloc(sizeof(*task));
    if (!task)
    {
        return 0;
    }
    memset(task, 0, sizeof(*task));
    task->pid = alloc_pid();
    task->parent = parent;
    task->state = TASK_NEW;
    uint8_t *stack = (uint8_t *) kmalloc(PROCESS_KERNEL_STACK_SIZE);
    if (!stack)
    {
        kfree(task);
        return 0;
    }
    task->kernel_stack_base = (uintptr_t) stack;
    task->kernel_stack_top = (uintptr_t) (stack + PROCESS_KERNEL_STACK_SIZE);
    mm_t *mm = (mm_t *) kmalloc(sizeof(*mm));
    if (!mm)
    {
        kfree(stack);
        kfree(task);
        return 0;
    }
    if (!create_task_mm(mm))
    {
        kfree(mm);
        kfree(stack);
        kfree(task);
        return 0;
    }
    task->mm = mm;
    task->cr3 = get_pd_lma((uintptr_t) mm->pgd);
    return task;
}

void destroy_task(task_t *task)
{
    release_task_mm(task);
    if (task->kernel_stack_base)
    {
        kfree((void *) task->kernel_stack_base);
    }
    kfree(task);
}

uint8_t prepare_user_task_stack(task_t *task, uint32_t entry_point, uint32_t user_eflags, uint32_t user_esp)
{
    uint32_t *stack = (uint32_t *) task->kernel_stack_top;
    *--stack = (uint32_t) GDT_USER_DATA_SELECTOR;
    *--stack = user_esp;
    *--stack = user_eflags;
    *--stack = (uint32_t) GDT_USER_CODE_SELECTOR;
    *--stack = entry_point;
    *--stack = 0;
    *--stack = (uint32_t) enter_user_task;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    task->kernel_esp = (uintptr_t) stack;
    return 1;
}

__attribute__((noreturn)) void spin_forever(void)
{
    for (;;);
}
