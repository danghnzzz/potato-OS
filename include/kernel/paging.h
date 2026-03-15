#ifndef PAGING_H
#define PAGING_H

#include <stddef.h>
#include <stdint.h>
#include <kernel/memory.h>

#define PAGING_DIRECTORY_POOL_SIZE 16
#define PAGING_TABLE_POOL_SIZE 64
#define PAGE_DIRECTORY_ENTRIES 1024
#define PAGE_TABLE_ENTRIES 1024
#define PAGE_SIZE 4096
#define KERNEL_VMA_TO_LMA_OFFSET 0xc0000000
#define PAGE_FLAG_PRESENT 0x1
#define PAGE_FLAG_WRITABLE 0x2
#define PAGE_FLAG_USER 0x4
#define PAGE_FLAG_WRITE_THROUGH 0x8
#define PAGE_FLAG_CACHE_DISABLE 0x10
#define PAGE_FLAG_ACCESSED 0x20
#define PAGE_FLAG_DIRTY 0x40
#define PAGE_FLAG_GLOBAL 0x100
#define PAGE_ADDRESS_MASK 0xfffff000
#define PAGE_FLAGS_MASK 0xfff

uintptr_t *alloc_pd(void);
uint8_t free_pd(uintptr_t *pgd);
uintptr_t *alloc_pt(void);
uint8_t free_pt(uintptr_t *pgt);
uintptr_t get_cr3(void);
void set_cr3(uintptr_t cr3);
uintptr_t get_pd_lma(uintptr_t pgd_virt);
uintptr_t get_pd_vma(uintptr_t pgd_phys);
uint8_t map_page(uintptr_t *pgd, uintptr_t virt_addr, uintptr_t phys_addr, uint32_t flags);
uint8_t unmap_page(uintptr_t *pgd, uintptr_t virt_addr);

#endif
