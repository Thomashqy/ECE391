#include "pcb.h"
#include "terminal.h"
#include "paging.h"
#include "act_syscall.h"
#include "lib.h"

int32_t error_func()
{
	return -1;
}

func_pointer stdin_table[NUM_FOPS] = {terminal_read, error_func, error_func};
func_pointer stdout_table[NUM_FOPS] = {error_func, terminal_write, error_func};
/*
    get_pcb
    Desc: Gets the pcb
    Inputs: none
    Return: pointer to specified pcb
    Side Effects: none
*/
pcb_t* get_pcb()
{
	uint32_t retval;
	asm volatile("movl %%esp, %0"  \
				  : "=m" (retval)  \
				  :  			   \
				  :	"memory"       );
	retval &= PCB_MASK;
	return (pcb_t*) retval;
}
/*
    init_pcb
    Desc: Initializes pcb for specified fields
    Inputs: new_pcb-pointer to a new pcb, next_pid- next process id, next_kernel_stack_start- start of next kernel stack
    Return: none
    Side Effects: modifies kernel page, modifies memory
*/
void init_pcb(pcb_t* new_pcb, uint32_t next_pid, uint32_t next_kernel_stack_start)
{
	int i;
	new_pcb->pid = next_pid;
	
	//stdin and stdout go here
	new_pcb->descriptors[0].fopstp = stdin_table;
	new_pcb->descriptors[1].fopstp = stdout_table;

	new_pcb->descriptors[0].f_flags = 1;
	new_pcb->descriptors[1].f_flags = 1;

	terminal_open(next_pid);

	for(i = MIN_NUM_FD; i < MAX_NUM_FD; i++)
		new_pcb->descriptors[i].f_flags = 0;

	new_pcb->parent_pde = get_pde(USER_VIRT_PAGE);
	
	new_pcb->f_esp0 = next_kernel_stack_start;
	asm volatile ("movl %%esp, %0"             \
	            : "=m" (new_pcb->parent_esp0)  \
				:                              \
				: "memory");
	asm volatile ("movl %%ebp, %0"             \
	            : "=m" (new_pcb->parent_ebp0)  \
				:                              \
				: "memory");
	asm volatile ("movl %%ebx, %0"             \
	            : "=m" (new_pcb->parent_ebx0)  \
				:                              \
				: "memory");
	asm volatile ("movl %%edi, %0"             \
	            : "=m" (new_pcb->parent_edi0)  \
				:                              \
				: "memory");
	asm volatile ("movl %%esi, %0"             \
	            : "=m" (new_pcb->parent_esi0)  \
				:                              \
				: "memory");

	memset(new_pcb->argument, '\0', MAX_ARG_BYTES);
}
