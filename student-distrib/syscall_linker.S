/* syscall_linker.S - The assembly linkage for system calls */

#define ASM     1

.text

.global syscall_linker

.align 4
syscall_linker:
	pushl    $0x0
	pushw     %fs
	pushw     %es
	pushw     %ds
	pushl     %ebp
	pushl     %edi
	pushl     %esi
	pushl     %edx
	pushl     %ecx
	pushl     %ebx

	decl %eax
	cmpl $9, %eax #check if actual system call num
	jbe sys_jump
	
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popw %ds
	popw %es
	popw %fs
	addl $4, %esp
	
	movl $-1, %eax

	iret
	
	sys_jump:
	
	# Arguments for system call
	pushl %edx
	pushl %ecx
	pushl %ebx
	
	call *syscall_table(,%eax,4)
	
	addl $12, %esp
	
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popw %ds
	popw %es
	popw %fs
	addl $4, %esp

	iret
	
	syscall_table:
	.long halt
	.long execute
	.long read
	.long write
	.long open
	.long close
	.long getargs
	.long vidmap
	.long set_handler
	.long sigreturn


