#include <stddef.h>
#include <stdint.h>
#include <kernel/elf.h>

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
