#include <stdint.h>
#include <kernel/paging.h>

static uintptr_t page_directory[PAGE_DIRECTORY_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uintptr_t first_page_table[PAGE_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

static void setup_first_page_identity_map(void)
{
    for (uint32_t i = 0; i < PAGE_DIRECTORY_ENTRIES; i++)
    {
        page_directory[i] = 0;
    }
    for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++)
    {
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE;
    }
    page_directory[0] = ((uint32_t) first_page_table) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE;
}

void enable_paging(void)
{
    setup_first_page_identity_map();
    __asm__ volatile
    (
        "mov cr3, %0"
        :
        : "r"(page_directory)
        : "memory"
    );
    uint32_t cr0;
    __asm__ volatile
    (
        "mov %0, cr0"
        : "=r"(cr0)
        :
        :
    );
    cr0 |= 0x80000000;
    __asm__ volatile
    (
        "mov cr0, %0"
        :
        : "r"(cr0)
        : "memory"
    );
}
