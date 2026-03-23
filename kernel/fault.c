#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <kernel/console.h>
#include <kernel/exceptions.h>
#include <kernel/fault.h>
#include <kernel/fs.h>
#include <kernel/memory.h>
#include <kernel/paging.h>
#include <kernel/process.h>

static uintptr_t read_cr2(void)
{
    uintptr_t fault_addr;

    __asm__ volatile(
        "mov %0, cr2\n"
        : "=r"(fault_addr)
        :
        : "memory"
    );
    return fault_addr;
}

static void page_fault_panic(void)
{
    console_puts("Unhandled page fault\n");
    for (;;);
}

static vm_area_t *find_vma(mm_t *mm, uintptr_t addr)
{
    vm_area_t *vma;
    for (vma = mm->mmap; vma; vma = vma->vm_next)
    {
        if (addr >= vma->vm_start && addr < vma->vm_end)
        {
            return vma;
        }
    }
    return 0;
}

static void page_fault_handler(uint8_t exc, uint32_t err_code)
{
    task_t *task = get_current_task();
    if (!task || !task->mm)
    {
        page_fault_panic();
    }
    if (err_code & 0x1)
    {
        page_fault_panic();
    }
    uintptr_t fault_addr = read_cr2();
    vm_area_t *vma = find_vma(task->mm, fault_addr);
    if (!vma)
    {
        page_fault_panic();
    }
    uintptr_t page_addr = fault_addr & PAGE_ADDRESS_MASK;
    uintptr_t phys_addr = alloc_phys_frame();
    if (!phys_addr)
    {
        page_fault_panic();
    }
    uint32_t pde_index = (page_addr >> 22) & 0x3ff;
    if (!(task->mm->pgd[pde_index] & PAGE_FLAG_PRESENT))
    {
        uintptr_t *page_table = alloc_pt();
        if (!page_table)
        {
            free_phys_frame(phys_addr);
            page_fault_panic();
        }
        task->mm->pgd[pde_index] = get_pd_lma((uintptr_t) page_table) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE | PAGE_FLAG_USER;
    }
    uint32_t flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE | PAGE_FLAG_USER;
    if (!map_page(task->mm->pgd, page_addr, phys_addr, flags))
    {
        free_phys_frame(phys_addr);
        page_fault_panic();
    }
    void *page_buf = (void *) page_addr;
    memset(page_buf, 0, PAGE_SIZE);
    if (vma->vm_file)
    {
        uint32_t file_offset = vma->vm_offset + (uint32_t) (page_addr - vma->vm_start);
        uint32_t file_size = vma->vm_file->f_inode->i_size;
        if (file_offset < file_size)
        {
            uint32_t bytes_to_read = (file_size - file_offset) < PAGE_SIZE ? (file_size - file_offset) : PAGE_SIZE;
            uint32_t current_pos = vma->vm_file->f_pos;
            vma->vm_file->f_pos = file_offset;
            if (vfs_read(vma->vm_file, page_buf, bytes_to_read) != (ssize_t) bytes_to_read)
            {
                vma->vm_file->f_pos = current_pos;
                unmap_page(task->mm->pgd, page_addr);
                free_phys_frame(phys_addr);
                page_fault_panic();
            }
            vma->vm_file->f_pos = current_pos;
        }
    }
}

uint8_t init_fault(void)
{
    console_puts("Setting up page fault handler ... ");
    register_exception_handler(14, page_fault_handler);
    console_puts("Done\n");
    return 1;
}
