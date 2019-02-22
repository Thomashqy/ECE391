/* System call functions. */
#include "act_syscall.h"
#include "paging.h"
#include "x86_desc.h"
#include "terminal.h"
#include "lib.h"
#include "pcb.h"
#include "rtc.h"

/* REALLY BIG COMMENT: make these functions return something. */
func_pointer rtc_table[NUM_FOPS] = {rtc_read, rtc_write, rtc_close};
func_pointer filesystem_table[NUM_FOPS] = {filesystem_read, filesystem_write, filesystem_close};

int32_t error_func2()
{
	return -1;
}

static uint32_t next_physical_addr = USER_PHYS_START;
static uint32_t next_kernel_stack_start = USER_PHYS_START - KERNEL_STACK_OFFSET - KERNEL_STACK_SIZE;
static uint32_t next_pid = 0;
/*
	Halt
	Desc: Halts the current executing process, returns all child data and registers to previous state, never returns, always jumps to execute
	Inputs: status, return to the parent 
	Return: 0
	Side Effects: halts the program from executing, restores parent paging
*/
extern int32_t halt(uint8_t status)
{
	pcb_t* pcb = get_pcb();
	uint8_t i;

	next_pid--;
	next_physical_addr -= PROG_PAGE_SIZE;
	next_kernel_stack_start += KERNEL_STACK_SIZE;

	set_pde(pcb->parent_pde, USER_VIRT_PAGE >> PDE_SHIFT);

	for(i = MIN_NUM_FD; i < MAX_NUM_FD; i++)
		if(pcb->descriptors[i].f_flags == 1)
			pcb->descriptors[i].fopstp[FOPS_CLOSE]();

	tss.esp0 = pcb->parent_esp0;
	// Stack Segment in TSS?

	asm volatile("movl  $0x0018, %%eax \n\
				  movw   %%ax, %%ds    \n \
				  movw   %%ax, %%es    \n \
				  movw   %%ax, %%fs    \n \
				  movw   %%ax, %%gs    \n \
				  xorl %%eax, %%eax   \n \
		          movb %0, %%al   \n  \
		          movl %1, %%esp   \n  \
		          movl %2, %%ebp   \n  \
		          jmp execute_return"  \
		          :                    \
		          : "m" (status), "m" (pcb->parent_esp0), "m" (pcb->parent_ebp0)  \
		          : "eax", "esp", "ebp");

	return 0;
}

int32_t halt_by_exception()
{
	pcb_t* pcb = get_pcb();
	uint8_t i;

	next_pid--;
	next_physical_addr -= PROG_PAGE_SIZE;
	next_kernel_stack_start += KERNEL_STACK_SIZE;

	set_pde(pcb->parent_pde, USER_VIRT_PAGE >> PDE_SHIFT);

	for(i = MIN_NUM_FD; i < MAX_NUM_FD; i++)
		if(pcb->descriptors[i].f_flags == 1)
			pcb->descriptors[i].fopstp[FOPS_CLOSE]();

	tss.esp0 = pcb->parent_esp0;
	// Stack Segment in TSS?

	asm volatile("movl  $0x0018, %%eax \n\
				  movw   %%ax, %%ds    \n \
				  movw   %%ax, %%es    \n \
				  movw   %%ax, %%fs    \n \
				  movw   %%ax, %%gs    \n \
				  movl $256, %%eax   \n \
		          movl %0, %%esp   \n  \
		          movl %1, %%ebp   \n  \
		          jmp execute_return"  \
		          :                    \
		          : "m" (pcb->parent_esp0), "m" (pcb->parent_ebp0)  \
		          : "eax", "esp", "ebp");

	return 0;
}

/*
	Execute
	Desc: Executes a process.
	Inputs: command, string that includes a file to attempt to execute 
	Return: -1 on error, or process status after executing
	Side Effects: executes program, changes paging structure, context switch to user space
*/
extern int32_t execute(const uint8_t* command)
{
	if(next_pid >= MAX_NUM_TASKS)
		return -1;

	int start = 0;
	int idx = 0;
	int arg_idx;
	uint8_t file_header[FILE_HEADER_SIZE];
	uint32_t file_length;
	uint8_t exe_name[FILE_NAME_MAX_NUM_CHAR + 1];
	pcb_t* new_pcb;
	dentry_t file_dentry;
	
	/* Strip leading spaces. */
	while(command[start] == ' ')
		start++;
	
	/* Find end of executable name. */  
	while(command[idx+start] != '\0' && command[idx+start] != ' ') {
		exe_name[idx] = command[idx+start];
	 	idx++;
	}
	exe_name[idx] = '\0';

	/* Find start of arguments. */
	for(arg_idx = idx+start; command[arg_idx] == ' '; arg_idx++) {}
	
	/* Read file header if file exists. */
	if(read_dentry_by_name(exe_name, &file_dentry) == -1)
	{
		printf("File not found\n");
		return -1;
	}
	
	/* Read file header. */
	read_data(file_dentry.inode_idx, 0, file_header, FILE_HEADER_SIZE);
	
	/* Check for executable. */
	if(check_executable(file_header))
	{
		printf("File not executable\n");
		return -1;
	}
	
	file_length = get_length(file_dentry.inode_idx);

	new_pcb = (pcb_t*)(next_kernel_stack_start & PCB_MASK);
	init_pcb(new_pcb, next_pid, next_kernel_stack_start);

	idx = 0;
	while(command[arg_idx] != '\0') {
		new_pcb->argument[idx] = command[arg_idx];
		arg_idx++;
		idx++;
	}

	next_pid++;
	next_kernel_stack_start -= KERNEL_STACK_SIZE;
	
	/* Set up new page for task. */
	uint32_t virt_addr = read_virt_addr(file_header);
	unsigned int flags = USER_PAGE_FLAGS;
	map_page_4mB((void*)next_physical_addr, (void*)USER_VIRT_PAGE, flags);
	next_physical_addr += PROG_PAGE_SIZE;
	
	/* Copy task data into new page. */
	read_data(file_dentry.inode_idx, 0, (uint8_t*)USER_COPY_START, file_length);

	tss.esp0 = new_pcb->f_esp0;
    tss.ss0 = KERNEL_DS;

    uint32_t retval;

	/* Push iret context in this order:
	 * User data segment, user stack pointer,
	 * eflags, user code segment, entry address. 
	 */
	asm volatile("movl  $0x002B, %%eax \n\
				  movw   %%ax, %%ds    \n \
				  movw   %%ax, %%es    \n \
				  movw   %%ax, %%fs    \n \
				  movw   %%ax, %%gs    \n \
				  movl %%esp, %1     \n \
				  movl %%ebp, %2     \n \
				  pushl $0x002B      \n \
	              pushl %4           \n \
				  pushf              \n \
				  orl    $0x200, (%%esp)  \n \
				  pushl $0x0023      \n \
				  pushl %3          \n \
				  iret              \n \
				  execute_return:   \n \
				  movl %%eax, %0"      \
				  : "=r" (retval), "=m" (new_pcb->parent_esp0), "=m" (new_pcb->parent_ebp0)  \
				  : "r" (virt_addr), "r" (USER_VIRT_PAGE + PROG_PAGE_SIZE)    \
				  : "memory", "cc", "eax");

	return retval;
}

/*
	Read
	Desc: Reads data from a specified file description into te given buffer, returning bytes read
	Inputs: fd-file descriptor, buf- buffer to read data into, nbytes-number of bytes  
	Return: number of bytes read
	Side Effects: file positions in pcb are updated
*/
extern int32_t read(int32_t fd, void* buf, int32_t nbytes)
{
	if(fd >= MAX_NUM_FD || fd < 0 || buf == NULL || nbytes < 0)
		return -1;

	pcb_t* pcb = get_pcb();
	if(pcb->descriptors[fd].f_flags != 1)
		return -1;

	return pcb->descriptors[fd].fopstp[FOPS_READ](fd, buf, nbytes);
}
 
/*
	Write
	Desc: Reads data from a specified file description into te given buffer, returning bytes read
	Inputs: fd-file descriptor, buf- buffer to read data into, nbytes-number of bytes  
	Return: number of bytes written or -1 on error
	Side Effects: may write to display, may changes rate of rtc
*/
extern int32_t write(int32_t fd, void* buf, int32_t nbytes)
{
	if(fd >= MAX_NUM_FD || fd < 0 || buf == NULL || nbytes < 0)
		return -1;

	pcb_t* pcb = get_pcb();
	if(pcb->descriptors[fd].f_flags != 1)
		return -1;

	return pcb->descriptors[fd].fopstp[FOPS_WRITE](fd, buf, nbytes);
}

/*
	open
	Desc: opens a file for a process
	Inputs: filename to open
	Return: returns -1 on fail, or returns file descriptor index
	Side Effects: opens file on success, initializing file descriptor entry and pcb, opening rtc resets rate to 2Hz
*/
extern int32_t open(const uint8_t* filename)
{

	dentry_t fd;
	pcb_t* control_block;

	if(read_dentry_by_name(filename, &fd) == -1)
		return -1;

	control_block = get_pcb();

	int32_t i;
	for(i = MIN_NUM_FD; i < MAX_NUM_FD; i++)
	{
		if(control_block->descriptors[i].f_flags == 0)
			break;
	}

	if(i == MAX_NUM_FD)
		return -1;

	switch(fd.file_type)
	{
		case RTC_TYPE: //RTC 
			control_block->descriptors[i].fopstp = rtc_table;
			control_block->descriptors[i].f_inode = NULL;
			rtc_open();
			break;
		case DIR_TYPE: //Directory
			control_block->descriptors[i].fopstp = filesystem_table;
			control_block->descriptors[i].f_inode = NULL;
			break;
		case REG_FILE_TYPE: //Regular File
			control_block->descriptors[i].fopstp = filesystem_table;
			control_block->descriptors[i].f_inode = inode_array + fd.inode_idx;
			break;
	}
	control_block->descriptors[i].f_position = 0;
	control_block->descriptors[i].f_flags = 1;

	return i;
}

/*
	Close
	Desc: closes a file
	Inputs: fd-file descriptor  
	Return: 0 on success, -1 on failure
	Side Effects: on success, closes file, makes file descriptor entry available in pcb.
*/
extern int32_t close(int32_t fd)
{
	if(fd >= MAX_NUM_FD || fd < MIN_NUM_FD)
		return -1;

	pcb_t* pcb = get_pcb();
	if(pcb->descriptors[fd].f_flags == 0)
		return -1;

	pcb->descriptors[fd].f_flags = 0;

	return pcb->descriptors[fd].fopstp[FOPS_CLOSE]();
}

extern int32_t getargs(uint8_t* buf, int32_t nbytes)
{
	pcb_t* pcb = get_pcb();
	uint32_t arg_len = strlen((int8_t*) pcb->argument);

	/* Check if arguments can fit in buffer. */
	if(nbytes < arg_len + 1)
		return -1;

	/* Copy arguments into buffer. */
	strcpy((int8_t*) buf, (int8_t*) pcb->argument);
	return 0;
}

extern int32_t vidmap(uint8_t** screen_start)
{
	if((uint32_t)screen_start < USER_VIRT_PAGE || (uint32_t)screen_start >= (USER_VIRT_PAGE + PROG_PAGE_SIZE))
		return -1;

	pcb_t* pcb = get_pcb();

	uint32_t pid = pcb->pid;

	map_to_vidmem((void*)(((USER_VIRT_PAGE + PROG_PAGE_SIZE))+PAGE_SIZE*pid), USER_READ_PRESENT);

	*screen_start = (uint8_t*)((USER_VIRT_PAGE + PROG_PAGE_SIZE)+PAGE_SIZE*pid);

	return 0;
}

extern int32_t set_handler(int32_t signum, void* handler_address)
{
	return -1;
}

extern int32_t sigreturn(void)
{
	return -1;
}



/*
	check_executable
	Desc: checks if the executable is valid
	Inputs: header
	Return: return 0 on success, -1 on failure
	Side Effects: none
*/
int
check_executable(uint8_t* header)
{
	uint8_t exe[EXE_MAGIC_NUM_SIZE] = {0x7F, 0x45, 0x4C, 0x46}; // These are the magic numbers that determine if a file is executable
	uint8_t i;
	
	for(i = 0; i < EXE_MAGIC_NUM_SIZE; i++)
		if(header[i] != exe[i])
			return -1;
		
	return 0;
}

/*
	read_virt_addr
	Desc: formats virtual address from header since it was stored in little-endian
	Inputs: header  
	Return: virtual address
	Side Effects: none
*/
uint32_t read_virt_addr(uint8_t* header)
{
	/* re-organize bytes 24-27 of the header into the virt addr */
	uint32_t virt_addr = header[24];

	virt_addr += header[25] << SHIFT_1_BYTE;

	virt_addr += header[26] << SHIFT_2_BYTE;

	virt_addr += header[27] << SHIFT_3_BYTE;

	return virt_addr;
}
