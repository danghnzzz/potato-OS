#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>
#include <kernel/fs.h>

typedef struct vm_area_struct_t vm_area_struct_t;
typedef struct
{
    uintptr_t *pgd;
    struct vm_area_struct_t *mmap;
} mm_t;
struct vm_area_struct_t
{
    uintptr_t vm_start;
    uintptr_t vm_end;
    uint32_t vm_offset;
    mm_t *vm_mm;
    file_t *vm_file;
    struct vm_area_struct_t *vm_prev;
    struct vm_area_struct_t *vm_next;
};
typedef struct vm_area_struct_t vm_area_t;

vm_area_t *do_mmap(uintptr_t addr, size_t length, file_t *file, uint32_t offset);

#endif
