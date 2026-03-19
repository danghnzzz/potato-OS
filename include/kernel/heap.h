#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>

#define KERNEL_HEAP_SIZE 65536

typedef struct heap_block_t
{
    size_t size;
    uint8_t is_free;
    struct heap_block_t *next;
} heap_block_t;

uint8_t init_heap(void);
void *kmalloc(size_t size);
void kfree(void *data_ptr);

#endif
