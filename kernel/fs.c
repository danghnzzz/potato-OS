#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <kernel/disk.h>
#include <kernel/fs.h>
#include <kernel/console.h>

static uint8_t super_block_buf[FS_BLOCK_SIZE];
static minix_super_block_t *super = (minix_super_block_t *) super_block_buf;

static uint8_t read_block(uint32_t block_idx, void *buffer)
{
    uint32_t lba = FILE_SYSTEM_BASE_SECTOR + (block_idx * SECTORS_PER_BLOCK);
    return disk_read(lba, buffer, SECTORS_PER_BLOCK);
}

static uint8_t read_zone_indirect_pointer(uint16_t indirect_zone, uint32_t idx, uint16_t *pointer_out)
{
    uint16_t block_buf[FS_BLOCK_SIZE / 2];
    if (!read_block(indirect_zone, block_buf))
    {
        return 0;
    }
    *pointer_out = block_buf[idx];
    return 1;
}

static uint8_t map_file_block(const minix_inode_t *inode, uint32_t zone_idx, uint32_t *disk_block_out)
{
    uint16_t zone;
    if (zone_idx < DIRECT_ZONES)
    {
        zone = inode->i_zone[zone_idx];
    }
    else if (zone_idx - DIRECT_ZONES < POINTERS_PER_ZONE)
    {
        uint16_t indirect_zone = inode->i_zone[INDIRECT_ZONE_INDEX];
        if (!read_zone_indirect_pointer(indirect_zone, zone_idx - DIRECT_ZONES, &zone))
        {
            return 0;
        }
    }
    else if (zone_idx - DIRECT_ZONES - POINTERS_PER_ZONE < POINTERS_PER_ZONE * POINTERS_PER_ZONE)
    {
        uint16_t lvl1_zone = inode->i_zone[DOUBLE_INDIRECT_ZONE_INDEX];
        uint32_t lvl1_idx = (zone_idx - DIRECT_ZONES - POINTERS_PER_ZONE) / POINTERS_PER_ZONE; 
        uint16_t lvl2_zone;
        uint32_t lvl2_idx = (zone_idx - DIRECT_ZONES - POINTERS_PER_ZONE) % POINTERS_PER_ZONE;
        if (!read_zone_indirect_pointer(lvl1_zone, lvl1_idx, &lvl2_zone))
        {
            return 0;
        }
        if (!read_zone_indirect_pointer(lvl2_zone, lvl2_idx, &zone))
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
    *disk_block_out = zone;
    return 1;
}

uint8_t init_fs(void)
{
    console_puts("Setting up file system ... ");
    uint32_t super_block_lba = FILE_SYSTEM_BASE_SECTOR + 2;
    if (!disk_read(super_block_lba, super_block_buf, (uint8_t) SECTORS_PER_BLOCK))
    {
        console_puts("Failed: can not read super block");
        return 0;
    }
    console_puts("Done\n");
    return 1;
}

ssize_t vfs_read(file_t *file, void *buffer, uint32_t count)
{
    minix_inode_t *inode = file->f_inode;
    uint32_t file_size = inode->i_size;
    uint32_t file_pos = file->f_pos;
    uint32_t remaining_in_file = file_size - file_pos;
    uint32_t max_bytes_to_read = remaining_in_file < count ? remaining_in_file : count;
    uint8_t block_buf[FS_BLOCK_SIZE];
    uint8_t *out = (uint8_t *) buffer;
    ssize_t read = 0;
    while (read < max_bytes_to_read)
    {
        uint32_t current_file_pos = file_pos + read;
        uint32_t file_block = current_file_pos / FS_BLOCK_SIZE;
        uint32_t block_offset = current_file_pos % FS_BLOCK_SIZE;
        uint32_t chunk_size = FS_BLOCK_SIZE - block_offset;
        uint32_t remaining = max_bytes_to_read - read;
        if (chunk_size > remaining)
        {
            chunk_size = remaining;
        }
        uint32_t disk_block;
        if (!map_file_block(inode, file_block, &disk_block))
        {
            return 0;
        }
        if (!read_block(disk_block, block_buf))
        {
            return 0;
        }
        memcpy(out + read, block_buf + block_offset, chunk_size);
        read += chunk_size;
    }
    file->f_pos += read;
    return read;
}
