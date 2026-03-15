#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <kernel/paging.h>

static uintptr_t pd_pool[PAGING_DIRECTORY_POOL_SIZE][PAGE_DIRECTORY_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uint8_t pd_in_use[PAGING_DIRECTORY_POOL_SIZE];
static uintptr_t pt_pool[PAGING_TABLE_POOL_SIZE][PAGE_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uint8_t pt_in_use[PAGING_TABLE_POOL_SIZE];

uintptr_t *alloc_pd(void)
{
    for (uint32_t i = 0; i < PAGING_DIRECTORY_POOL_SIZE; i++)
    {
        if (!pd_in_use[i])
        {
            pd_in_use[i] = 1;
            memset(pd_pool[i], 0, PAGE_SIZE);
            return pd_pool[i];
        }
    }
    return 0;
}

uint8_t free_pd(uintptr_t *pgd)
{
    for (uint32_t i = 0; i < PAGING_DIRECTORY_POOL_SIZE; i++)
    {
        if (pd_pool[i] == pgd)
        {
            pd_in_use[i] = 0;
            return 1;
        }
    }
    return 0;
}

uintptr_t *alloc_pt(void)
{
    for (uint32_t i = 0; i < PAGING_TABLE_POOL_SIZE; i++)
    {
        if (!pt_in_use[i])
        {
            pt_in_use[i] = 1;
            memset(pt_pool[i], 0, PAGE_SIZE);
            return pt_pool[i];
        }
    }
    return 0;
}

uint8_t free_pt(uintptr_t *pgt)
{
    for (uint32_t i = 0; i < PAGING_TABLE_POOL_SIZE; i++)
    {
        if (pt_pool[i] == pgt)
        {
            pt_in_use[i] = 0;
            return 1;
        }
    }
    return 0;
}

uintptr_t get_cr3(void)
{
    uintptr_t cr3;
    __asm__ volatile(
        "mov %0, cr3\n"
        : "=r"(cr3)
        :
        : "memory"
    );
    return cr3 & PAGE_ADDRESS_MASK;
}

void set_cr3(uintptr_t cr3)
{
    __asm__ volatile(
        "mov cr3, %0\n"
        :
        : "r"(cr3)
        : "memory"
    );
}

uintptr_t get_pd_lma(uintptr_t pgd_virt)
{
    return (pgd_virt - KERNEL_VMA_TO_LMA_OFFSET) & PAGE_ADDRESS_MASK;
}

uintptr_t get_pd_vma(uintptr_t pgd_phys)
{
    return (pgd_phys & PAGE_ADDRESS_MASK) + KERNEL_VMA_TO_LMA_OFFSET;
}

static void flush_tlb_single(uintptr_t vma)
{
    __asm__ volatile(
        "invlpg [%0]\n"
        :
        : "r"(vma)
        : "memory"
    );
}

uint8_t map_page(uintptr_t *pgd, uintptr_t page_base_addr, uintptr_t frame_base_addr, uint32_t flags)
{
    if (!(flags & PAGE_FLAG_PRESENT))
    {
        return 0;
    }
    uint16_t pde_index = (page_base_addr >> 22) & 0x3ff;
    uint16_t pte_index = (page_base_addr >> 12) & 0x3ff;
    uintptr_t *page_table = (uintptr_t *) get_pd_vma(pgd[pde_index]);
    if (!page_table)
    {
        return 0;
    }
    page_table[pte_index] = frame_base_addr | flags;
    if (get_cr3() == get_pd_lma((uintptr_t) pgd))
    {
        flush_tlb_single(page_base_addr);
    }
    return 1;
}

uint8_t unmap_page(uintptr_t *pgd, uintptr_t page_base_addr)
{
    uint16_t pde_index = (page_base_addr >> 22) & 0x3ff;
    uint16_t pte_index = (page_base_addr >> 12) & 0x3ff;
    uintptr_t pde = pgd[pde_index];
    if (!(pde & PAGE_FLAG_PRESENT))
    {
        return 0;
    }
    uintptr_t *page_table = (uintptr_t *) get_pd_vma(pde);
    page_table[pte_index] = 0;
    if (get_cr3() == get_pd_lma((uintptr_t) pgd))
    {
        flush_tlb_single(page_base_addr);
    }
    return 1;
}
