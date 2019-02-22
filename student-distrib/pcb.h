#ifndef PCB_H
#define PCB_H

#include "types.h"
#include "file_system.h"

#define PCB_MASK 0xFFFFE000
#define MAX_ARG_BYTES 128
#define MAX_NUM_DATABLOCKS_IN_INODE 1023 //First 4B of 4096B block is for the length of data

#define NUM_FOPS 3
#define FOPS_READ 0
#define FOPS_WRITE 1
#define FOPS_CLOSE 2
#define MAX_NUM_FD 8
#define MIN_NUM_FD 2


typedef int32_t (*func_pointer)(/*int32_t, void*, int32_t*/);

typedef struct fde_t {
	func_pointer* fopstp;
	i_node_t* f_inode;
	int32_t f_position;
	int32_t f_flags;
} fde_t;

// PCB structure
typedef struct pcb_t {
	uint32_t pid;
	fde_t descriptors[8];
	uint32_t f_esp0;
	uint32_t parent_pde;
	uint32_t parent_esp0;
	uint32_t parent_ebp0;
	uint32_t parent_ebx0;
	uint32_t parent_edi0;
	uint32_t parent_esi0;
	uint8_t argument[MAX_ARG_BYTES];
} pcb_t;

pcb_t* get_pcb();
void init_pcb(pcb_t* new_pcb, uint32_t next_pid, uint32_t next_kernel_stack_start);

#endif
