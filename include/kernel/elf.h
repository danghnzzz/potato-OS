#ifndef ELF_H
#define ELF_H

#include <stddef.h>
#include <stdint.h>
#include <kernel/fs.h>

#define ELF32_PT_LOAD 1

typedef struct __attribute__((packed))
{
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} elf32_ehdr_t;
typedef struct __attribute__((packed))
{
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} elf32_phdr_t;

uint8_t elf32_load_ehdr(file_t *file, elf32_ehdr_t *elf32_ehdr_out);
uint8_t elf32_load_phdr(file_t *file, elf32_phdr_t *elf32_phdr_out);
uint8_t elf32_load_exec(file_t *file, uint32_t *entry_point);

#endif
