#include <stddef.h>
#include <stdint.h>
#include <kernel/process.h>
#include <kernel/memory.h>
#include <kernel/paging.h>

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

__attribute__((noreturn)) void spin_forever(void)
{
    for (;;);
}
