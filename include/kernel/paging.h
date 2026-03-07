#ifndef PAGING_H
#define PAGING_H

#define PAGE_DIRECTORY_ENTRIES 1024
#define PAGE_TABLE_ENTRIES 1024
#define PAGE_SIZE 4096
#define PAGE_FLAG_PRESENT 0x1
#define PAGE_FLAG_WRITABLE 0x2
#define PAGE_FLAG_USER 0x4

uint8_t enable_paging(void);

#endif
