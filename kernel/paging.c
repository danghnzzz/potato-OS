#include <stdint.h>
#include <kernel/paging.h>

static uintptr_t page_directory[PAGE_DIRECTORY_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uintptr_t kernel_page_table[PAGE_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uintptr_t user_page_table[PAGE_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

static void clear_page_directory(void)
{
    for (uint32_t i = 0; i < PAGE_DIRECTORY_ENTRIES; i++)
    {
        page_directory[i] = 0;
    }
}

static void setup_kernel_first_page_identity_map(void)
{
    for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++)
    {
        kernel_page_table[i] = (i * PAGE_SIZE) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE;
    }
    page_directory[0] = ((uint32_t) kernel_page_table) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE;
}

static void setup_user_first_page(void)
{
    for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++)
    {
        user_page_table[i] = (0x400000 + (i * PAGE_SIZE)) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE | PAGE_FLAG_USER;
    }
    page_directory[1] = ((uint32_t) user_page_table) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE | PAGE_FLAG_USER;
}

void enable_paging(void)
{
    clear_page_directory();
    setup_kernel_first_page_identity_map();
    setup_user_first_page();
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
