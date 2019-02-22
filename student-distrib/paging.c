#include "paging.h"
#include "x86_desc.h"
#include "pcb.h"

uint32_t pde[PAGE_DIR_SIZE] __attribute__((aligned(BYTES_ALLIGN)));    //might be more???
uint32_t pte[PAGE_DIR_SIZE] __attribute__((aligned(BYTES_ALLIGN)));

uint32_t user_pt[PAGE_DIR_SIZE] __attribute__((aligned(BYTES_ALLIGN)));

/*
    Initializes paging by setting up the page directory and page tables.
    We set the video memory in the 0th byte of the page directory, and the 1st byte to the kernel. 
    We then enable paging by setting the cr3, cr0, and cr4 bits in the assembly code. We map the
    kernel to the physical addresses using the map_page function
*/

void init_paging(){
    int i;
    uint32_t vid_mem = VID_PHYS_MEM_START/PAGE_SIZE;   //finding index 

    for(i = 0; i < PAGE_DIR_SIZE; i++){
        pde[i] = READWRITE;
    }       
    
    pde[1] = 0x400183;      //page directory entry for kernel


    for(i = 0; i < PAGE_DIR_SIZE; i++){
        pte[i] = (i * PAGE_SIZE) | READWRITE;
    }
    pte[vid_mem] = VID_PHYS_MEM_START | READ_PRESENT;     //page table entry for video memory
    pde[0] = ((unsigned int) pte) | READ_PRESENT;      //initializing other page directory entries
	
	tss.cr3 = (uint32_t) pde;

     asm volatile(
        "movl $pde, %%eax     \n \
        movl %%eax, %%cr3                   \n \
        movl %%cr4, %%eax                   \n \
        orl $0x00000010, %%eax              \n \
        movl %%eax, %%cr4                   \n \
        movl %%cr0, %%eax                   \n \
        orl $0x80000000, %%eax              \n \
        movl %%eax, %%cr0"                  \
        :                                   \
        : "g"(pde)            \
        : "memory", "cc", "eax"); 
}

/*
    this function maps the physical address with the virtual address
    check whether the PD entry is present.
    create a new empty PT and
    adjust the PDE accordingly.
    check whether the PT entry is present.
    When it is, then there is already a mapping present. 
    flush the entry in the TLB
    or you might not notice the change.
*/
void map_to_vidmem(void * virtualaddr, unsigned int flags)
{
 
    unsigned long pdindex = (unsigned long)virtualaddr >> PDE_SHIFT;
    unsigned long ptindex = (unsigned long)virtualaddr >> PTE_SHIFT & PAGE_TABLE_MASK;
 
    if(!(pde[pdindex] & 0x1))
        pde[pdindex] = ((unsigned long)(user_pt) & PAGE_DIR_MASK) | USER_READ_PRESENT;
    
    user_pt[ptindex] = VID_PHYS_MEM_START | (flags & FLAG_MASK_4KB) | 0x01; // Present
}

/*
    this function maps the physical address with the virtual address
    check whether the PD entry is present.
    When it is, then there is already a mapping present. 
    Save entree based on pid, and flush the entry in the TLB.
*/
void map_page_4mB(void * physaddr, void * virtualaddr, unsigned int flags)
{
 
    unsigned long pdindex = (unsigned long)virtualaddr >> PDE_SHIFT;

	pde[pdindex] = (((unsigned long) physaddr) | (flags & FLAG_MASK_4MB) | 0x01);

    asm volatile(
        "movl %%cr3, %%eax           \n\
         movl %%eax, %%cr3"          \
        :                  \
		:                  \
		: "eax");

}

/*
    set_pde
    Desc: Sets the pde at specified index
    Inputs: entry-specified pde, index-specified index
    Return: none
    Side Effects: modifies paging, flushes tlb
*/
void set_pde(uint32_t entry, uint32_t index)
{
    pde[index] = entry;
    asm volatile(
        "movl %%cr3, %%eax           \n\
         movl %%eax, %%cr3"          \
        :                  \
        :                  \
        : "eax");
}
/*
    get_pde
    Desc: Gets the pde at virtual address
    Inputs: virtual address
    Return: specified pde
    Side Effects: none
*/
uint32_t get_pde(uint32_t virt_addr)
{
	uint32_t pdindex = virt_addr >> PDE_SHIFT;
	return pde[pdindex];
}
