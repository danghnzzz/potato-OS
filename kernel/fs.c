#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <kernel/heap.h>
#include <kernel/disk.h>
#include <kernel/fs.h>
#include <kernel/console.h>

static uint8_t super_block_buf[FS_BLOCK_SIZE];
static minix_super_block_t *super = (minix_super_block_t *) super_block_buf;

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

static size_t split_path(const char *path, char components[PATH_COMPONENTS][MINIX_MAX_NAME_LENGTH + 1])
{

    size_t name_i = 0;
    if (!path || path[0] != '/')
    {
        return 0;
    }
    while (*path)
    {
        while (*path == '/')
        {
            path++;
        }
        if (!*path)
        {
            break;
        }
        size_t char_i = 0;
        while (path[char_i] && path[char_i] != '/')
        {
            components[name_i][char_i] = path[char_i];
            char_i++;
        }
        components[name_i][char_i] = '\0';
        name_i++;
        path += char_i;
    }
    return name_i;
}

static uint8_t read_block(uint32_t block_idx, void *buffer)
{
    uint32_t lba = FILE_SYSTEM_BASE_SECTOR + (block_idx * SECTORS_PER_BLOCK);
    return disk_read(lba, buffer, SECTORS_PER_BLOCK);
}

static uint8_t read_inode(uint16_t inode_num, minix_inode_t *inode_out)
{
    uint16_t inode_idx = inode_num - 1;
    uint32_t block_idx = 2 + super->s_imap_blocks + super->s_zmap_blocks + (inode_idx / INODES_PER_BLOCK);
    uint32_t inode_offset = (inode_idx % INODES_PER_BLOCK) * sizeof(minix_inode_t);
    uint8_t block_buf[FS_BLOCK_SIZE];
    if (!read_block(block_idx, block_buf))
    {
        return 0;
    }
    memcpy(inode_out, block_buf + inode_offset, sizeof(*inode_out));
    return 1;
}

static uint8_t set_inode(file_t *file, const minix_inode_t *inode)
{
    minix_inode_t *file_inode = (minix_inode_t *) kmalloc(sizeof(minix_inode_t));
    memcpy(file_inode, inode, sizeof(*file_inode));
    file->f_inode = file_inode;
    file->f_pos = 0;
    return 1;
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
    if (zone == 0)
    {
        return 0;
    }
    *disk_block_out = zone;
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

static uint8_t is_directory(const minix_inode_t *inode)
{
    return inode && ((inode->i_mode & MINIX_MODE_MASK) == MINIX_MODE_DIRECTORY);
}

static uint8_t name_equal(char *name1, char*name2)
{
    if (!name1 || !name2)
    {
        return 0;
    }
    for (uint32_t i = 0; i < MINIX_MAX_NAME_LENGTH + 1; i++)
    {
        if (name1[i] != name2[i])
        {
            return 0;
        }
        if (!name1[i])
        {
            return !name2[i];
        }
    }
    return 0;
}

static uint16_t get_child_inode_num(const minix_inode_t *dir_inode, const char *name)
{
    file_t dir_file;
    uint8_t block_buf[FS_BLOCK_SIZE];
    const uint32_t dir_entry_size = 2 + MINIX_MAX_NAME_LENGTH;
    if (!dir_inode || !name || !is_directory(dir_inode))
    {
        return 0;
    }
    dir_file.f_inode = (minix_inode_t *) dir_inode;
    dir_file.f_pos = 0;
    while (dir_file.f_pos < dir_inode->i_size)
    {
        uint32_t remaining = dir_inode->i_size - dir_file.f_pos;
        uint32_t to_read = remaining < FS_BLOCK_SIZE ? remaining : FS_BLOCK_SIZE;
        ssize_t read = vfs_read(&dir_file, block_buf, to_read);
        if (!read)
        {
            return 0;
        }
        for (uint32_t entry_offset = 0; entry_offset + dir_entry_size <= (uint32_t) read; entry_offset += dir_entry_size)
        {
            char entry_name[MINIX_MAX_NAME_LENGTH + 1];
            uint16_t child_inode_num = (uint16_t) block_buf[entry_offset] | ((uint16_t) block_buf[entry_offset + 1] << 8);
            if (!child_inode_num)
            {
                continue;
            }
            for (uint32_t i = 0; i < MINIX_MAX_NAME_LENGTH; i++)
            {
                entry_name[i] = (char) block_buf[entry_offset + 2 + i];
            }
            entry_name[MINIX_MAX_NAME_LENGTH] = '\0';
            if (name_equal(entry_name, (char *) name))
            {
                return child_inode_num;
            }
        }
    }
    return 0;
}

uint8_t vfs_open(const char *path, file_t *file)
{
    minix_inode_t current_inode;
    char components[PATH_COMPONENTS][MINIX_MAX_NAME_LENGTH + 1];
    int32_t component_count = split_path(path, components);
    int32_t i;
    file->f_inode = 0;
    file->f_pos = 0;
    if (!read_inode(MINIX_ROOT_INODE, &current_inode))
    {
        return 0;
    }
    if (component_count == 0)
    {
        return set_inode(file, &current_inode);
    }
    for (i = 0; i < component_count; i++)
    {
        uint16_t child_inode_num = get_child_inode_num(&current_inode, components[i]);
        if (!child_inode_num)
        {
            return 0;
        }
        if (!read_inode(child_inode_num, &current_inode))
        {
            return 0;
        }
    }
    return set_inode(file, &current_inode);
}

void vfs_close(file_t *file)
{
    if (file->f_inode)
    {
        kfree(file->f_inode);
        file->f_inode = 0;
    }
    file->f_pos = 0;
}
