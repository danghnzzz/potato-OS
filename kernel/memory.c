#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <kernel/console.h>
#include <kernel/memory.h>
#include <kernel/paging.h>

static uint8_t physical_frame_in_use[PHYSICAL_FRAMES];

static void mask_kernel_pages(void)
{
    for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++)
    {
        physical_frame_in_use[i] = 1;
    }
}

uint8_t init_physical_memory_allocator(void)
{
    console_puts("Setting up physical memory allocator ... ");
    memset(physical_frame_in_use, 0, sizeof(physical_frame_in_use));
    mask_kernel_pages();
    console_puts("Done\n");
    return 1;
}

uintptr_t alloc_phys_frame(void)
{
    for (uint32_t i = 0; i < PHYSICAL_FRAMES; i++)
    {
        if (!physical_frame_in_use[i])
        {
            physical_frame_in_use[i] = 1;
            return (uintptr_t) (i * PAGE_SIZE);
        }
    }
    return 0;
}

uint8_t free_phys_frame(uintptr_t frame_base_addr)
{
    uint32_t frame_index = frame_base_addr / PAGE_SIZE;
    if (frame_index < PAGE_TABLE_ENTRIES)
    {
        return 0;
    }
    physical_frame_in_use[frame_index] = 0;
    return 1;
}
