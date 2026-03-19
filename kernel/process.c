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

static uint8_t map_proc1_page_table(uintptr_t *pgd)
{
    uint32_t pde_index = (0x00400000 >> 22) & 0x3ff;
    uintptr_t *pgt = alloc_pt();
    if (!pgt)
    {
        return 0;
    }
    pgd[pde_index] = get_pd_lma((uintptr_t) pgt) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE | PAGE_FLAG_USER;
    if (!map_page(pgd, 0x00400000, 0x00400000, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE | PAGE_FLAG_USER))
    {
        free_pt(pgt);
        pgd[pde_index] = 0;
        return 0;
    }
    return 1;
}

uint8_t create_process_mm(mm_t *mm)
{
    uintptr_t *pgd = alloc_pd();
    if (!pgd)
    {
        return 0;
    }
    uintptr_t *current_pgd = (uintptr_t *) get_pd_vma(get_cr3());
    copy_kernel_mappings(pgd, current_pgd);
    if (!map_proc1_page_table(pgd))
    {
        free_pd(pgd);
        return 0;
    }
    mm->pgd = pgd;
    mm->mmap = 0;
    return 1;
}
