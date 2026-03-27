#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <kernel/heap.h>
#include <kernel/memory.h>
#include <kernel/paging.h>
#include <kernel/process.h>

uintptr_t align_down(uintptr_t addr)
{
    return addr & ~(PAGE_SIZE - 1);
}

uintptr_t align_up(uintptr_t addr)
{
    return (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

uint8_t padzero(uintptr_t addr)
{
    task_t *task = get_current_task();
    size_t num_zeros = (size_t) (align_up(addr) - addr);
    uintptr_t current_cr3 = get_cr3();
    uintptr_t target_cr3 = get_pd_lma((uintptr_t) task->mm->pgd);
    if (current_cr3 != target_cr3)
    {
        set_cr3(target_cr3);
    }
    memset((void *) addr, 0, num_zeros);
    if (current_cr3 != target_cr3)
    {
        set_cr3(current_cr3);
    }
    return 1;
}

vm_area_t *do_mmap(uintptr_t addr, size_t length, file_t *file, uint32_t offset)
{
    task_t *task = get_current_task();
    mm_t *mm = task->mm;
    uintptr_t start = align_down(addr);
    if (file && offset < (uint32_t) (addr - start))
    {
        return 0;
    }
    uintptr_t end = align_up(addr + length);
    vm_area_t *vma = kmalloc(sizeof(vm_area_t));
    if (!vma)
    {
        return 0;
    }
    vma->vm_start = start;
    vma->vm_end = end;
    vma->vm_offset = file ? (offset - (uint32_t) (addr - start)) : 0;
    vma->vm_mm = mm;
    vma->vm_file = file;
    vma->vm_prev = 0;
    vma->vm_next = 0;
    if (!mm->mmap)
    {
        mm->mmap = vma;
        return vma;
    }
    vm_area_t *iter = mm->mmap;
    vm_area_t *prev = 0;
    while (iter && iter->vm_start < start)
    {
        prev = iter;
        iter = iter->vm_next;
    }
    if ((prev && prev->vm_end > start) || (iter && end > iter->vm_start))
    {
        kfree(vma);
        return 0;
    }
    vma->vm_prev = prev;
    vma->vm_next = iter;
    if (prev)
    {
        prev->vm_next = vma;
    }
    else
    {
        mm->mmap = vma;
    }
    if (iter)
    {
        iter->vm_prev = vma;
    }
    return vma;
}

vm_area_t *vm_brk_flags(uintptr_t addr, size_t length)
{
    return do_mmap(addr, length, 0, 0);
}
