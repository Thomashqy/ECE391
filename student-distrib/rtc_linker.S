/* rtc_linker.S - The assembly linkage to the RTC handler */

#define ASM     1

.text

.global rtc_linker

.align 4
rtc_linker:
	pushl    $0x0
	pushl    $0x8
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
	
	call rtc_handler
	
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
	addl $8, %esp

	iret
