#include <stddef.h>
#include <stdint.h>
#include <kernel/elf.h>
#include <kernel/heap.h>
#include <kernel/memory.h>

static uint8_t read_elf(file_t *file, uint32_t offset, void *buffer, uint32_t count)
{
    uint32_t current_pos = file->f_pos;
    file->f_pos = offset;
    if (vfs_read(file, buffer, count) != (ssize_t) count)
    {
        file->f_pos = current_pos;
        return 0;
    }
    file->f_pos = current_pos;
    return 1;
}

uint8_t elf32_load_ehdr(file_t *file, elf32_ehdr_t *elf32_ehdr_out)
{
    if (!read_elf(file, 0, elf32_ehdr_out, sizeof(*elf32_ehdr_out)))
    {
        return 0;
    }
    return 1;
}

uint8_t elf32_load_phdr(file_t *file, elf32_phdr_t *elf32_phdr_out)
{
    elf32_ehdr_t ehdr;
    if (!elf32_load_ehdr(file, &ehdr))
    {
        return 0;
    }
    uint32_t phdr_bytes = ehdr.e_phnum * sizeof(elf32_phdr_t);
    if (!phdr_bytes)
    {
        return 1;
    }
    return read_elf(file, ehdr.e_phoff, elf32_phdr_out, phdr_bytes);
}

uint8_t elf32_load_exec(file_t *file, uint32_t *entry_point)
{
    elf32_ehdr_t ehdr;
    if (!elf32_load_ehdr(file, &ehdr))
    {
        return 0;
    }
    if (!ehdr.e_phnum)
    {
        return 0;
    }
    elf32_phdr_t phdr[ehdr.e_phnum];
    if (!phdr)
    {
        return 0;
    }
    if (!elf32_load_phdr(file, phdr))
    {
        kfree(phdr);
        return 0;
    }
    for (uint32_t i = 0; i < ehdr.e_phnum; i++)
    {
        if (phdr[i].p_type != ELF32_PT_LOAD)
        {
            continue;
        }
        if (phdr[i].p_memsz < phdr[i].p_filesz)
        {
            kfree(phdr);
            return 0;
        }
        if (!phdr[i].p_filesz)
        {
            if (phdr[i].p_memsz && !vm_brk_flags(phdr[i].p_vaddr, phdr[i].p_memsz))
            {
                kfree(phdr);
                return 0;
            }
            continue;
        }
        if (!do_mmap(phdr[i].p_vaddr, phdr[i].p_filesz, file, phdr[i].p_offset))
        {
            kfree(phdr);
            return 0;
        }
        if (phdr[i].p_memsz > phdr[i].p_filesz && !padzero(phdr[i].p_vaddr + phdr[i].p_filesz))
        {
            kfree(phdr);
            return 0;
        }
        uintptr_t anonymous_zeros_start = align_up(phdr[i].p_vaddr + phdr[i].p_filesz);
        uintptr_t anonymous_zeros_end = phdr[i].p_vaddr + phdr[i].p_memsz;
        if (anonymous_zeros_end > anonymous_zeros_start && !vm_brk_flags(anonymous_zeros_start, anonymous_zeros_end - anonymous_zeros_start))
        {
            kfree(phdr);
            return 0;
        }
    }
    *entry_point = ehdr.e_entry;
    return 1;
}
