/* act_syscall_linker.S - The assembly linkage for actual system calls */

#define ASM     1

.text

.global execute, halt, read, write, open, close, getargs , vidmap, set_handler, sigreturn 
