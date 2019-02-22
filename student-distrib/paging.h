#include "types.h"

#ifndef _PAGING_H
#define _PAGING_H

#define PAGE_DIR_SIZE 1024
#define PAGE_SIZE 0x1000
#define BYTES_ALLIGN 4096
#define READWRITE 0x00000002
#define USER_READ_PRESENT 0x07
#define READ_PRESENT 0x03

#define VID_PHYS_MEM_START 0xB8000
#define PDE_SHIFT 22
#define PTE_SHIFT 12
#define FLAG_MASK_4KB 0xFFF
#define FLAG_MASK_4MB 0x1FFF
#define PAGE_TABLE_MASK 0x03FF
#define PAGE_DIR_MASK 0xFFFFF000

void loadPageDirectory(uint32_t);
void enablePaging();
void init_paging();

void map_page_4mB(void * physaddr, void * virtualaddr, unsigned int flags);
void map_to_vidmem(void * virtualaddr, unsigned int flags);

void set_pde(uint32_t entry, uint32_t index);
uint32_t get_pde(uint32_t virt_addr);

#endif /* _PAGING_H */
