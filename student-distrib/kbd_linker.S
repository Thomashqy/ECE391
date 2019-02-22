/* kbd_linker.S - The assembly linkage to the KBD handler */

#define ASM     1

.text

.global kbd_linker

.align 4
kbd_linker:
	pushl    $0x0
	pushl    $0x8 #IRQ num of kbd
	push     %fs
	push     %es
	push     %ds
	push     %eax
	push     %ebp
	push     %edi
	push     %esi
	push     %edx
	push     %ecx
	push     %ebx
	
	call kbd_handler
	
	pop %ebx
	pop %ecx
	pop %edx
	pop %esi
	pop %edi
	pop %ebp
	pop %eax
	pop %ds
	pop %es
	pop %fs
	addl $8, %esp #popping two values off the stack

	iret
