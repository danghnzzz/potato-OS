#include <stddef.h>
#include <stdint.h>
#include <kernel/heap.h>
#include <kernel/console.h>

static uint8_t kernel_heap[KERNEL_HEAP_SIZE] __attribute__((aligned(8)));
static heap_block_t *heap_head;

uint8_t init_heap(void)
{
    console_puts("Setting up kernel heap space ... ");
    heap_head = (heap_block_t *) kernel_heap;
    heap_head->size = KERNEL_HEAP_SIZE - sizeof(heap_block_t);
    heap_head->is_free = 1;
    heap_head->next = 0;
    console_puts("Done\n");
    return 1;
}

static void split_block(heap_block_t *block, size_t size)
{
    heap_block_t *new_block;
    if (block->size < size + sizeof(heap_block_t) + 8)
    {
        return;
    }
    new_block = (heap_block_t *) ((uint8_t *) block + sizeof(heap_block_t) + size);
    new_block->size = block->size - size - sizeof(heap_block_t);
    new_block->is_free = 1;
    new_block->next = block->next;
    block->size = size;
    block->next = new_block;
}

static size_t align_up(size_t size)
{
    return (size + 0x7) & 0xfffffff8;
}

void *kmalloc(size_t size)
{
    heap_block_t *block;
    size = align_up(size);
    for (block = heap_head; block; block = block->next)
    {
        if (block->is_free && block->size >= size)
        {
            split_block(block, size);
            block->is_free = 0;
            return (uint8_t *) block + sizeof(heap_block_t);
        }
    }
    return 0;
}

static void coalesce_free_blocks(void)
{
    heap_block_t *block = heap_head;
    while (block && block->next)
    {
        uintptr_t block_consec = (uintptr_t) ((uint8_t *) block + sizeof(heap_block_t) + block->size);
        if (block->is_free && block->next->is_free && block_consec == (uintptr_t) block->next)
        {
            block->size += sizeof(heap_block_t) + block->next->size;
            block->next = block->next->next;
            continue;
        }
        block = block->next;
    }
}

void kfree(void *data_ptr)
{
    heap_block_t *block;
    block = (heap_block_t *) ((uint8_t *) data_ptr - sizeof(heap_block_t));
    block->is_free = 1;
    coalesce_free_blocks();
}
