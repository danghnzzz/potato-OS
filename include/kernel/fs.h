#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <stdint.h>
#include <kernel/disk.h>

#define FILE_SYSTEM_BASE_SECTOR 2048
#define FS_BLOCK_SIZE 1024
#define SECTORS_PER_BLOCK (FS_BLOCK_SIZE / DISK_SECTOR_SIZE)
#define DIRECT_ZONES 7
#define INDIRECT_ZONE_INDEX 7
#define DOUBLE_INDIRECT_ZONE_INDEX 8
#define POINTERS_PER_ZONE 512

typedef struct __attribute__((packed))
{
    uint16_t s_ninodes;
    uint16_t s_nzones;
    uint16_t s_imap_blocks;
    uint16_t s_zmap_blocks;
    uint16_t s_firstdatazone;
    uint16_t s_log_zone_size;
    uint32_t s_max_size;
    uint16_t s_magic;
    uint16_t s_state;
} minix_super_block_t;
typedef struct __attribute__((packed))
{
    uint16_t i_mode;
    uint16_t i_uid;
    uint32_t i_size;
    uint32_t i_time;
    uint8_t i_gid;
    uint8_t i_nlinks;
    uint16_t i_zone[9];
} minix_inode_t;
typedef struct __attribute__((packed))
{
	minix_inode_t *f_inode;
	uint32_t f_pos;
} file_t;

uint8_t init_fs(void);
ssize_t vfs_read(file_t *file, void *buffer, uint32_t count);

#endif
