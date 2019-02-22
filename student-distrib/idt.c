#include "idt.h"
#include "rtc.h"
#include "terminal.h"
#include "rtc_linker.h"
#include "kbd_linker.h"
#include "syscall_linker.h"
#include "paging.h"
#include "act_syscall.h"

/*
 * init_idt()
 *
 * Description:
 * Initializes the Interrupt Descriptor Table.
 * In order to initialize the IDT we fill in the values of the array of struct
 * with the values required of the interruption descriptor as defined in the 
 * ISA manual.
 *
 */
void init_idt ()
{
	int index;

	/* Load IDT size and base address to the IDTR */
	lidt(idt_desc_ptr);
    											
	for(index = 0; index < NUM_VEC; index++)
	{

		/* Set interruption vector present */
		idt[index].present = 0x1;

		/* Set privilege level to 0 */
		idt[index].dpl = KERNEL_PRIVILEGE;

		/* All vectors less than 32 are initialized as exceptions */				
		idt[index].size = 0x1;
		idt[index].reserved0 = 0x0;
		idt[index].reserved1 = 0x1;
		idt[index].reserved2 = 0x1;
		idt[index].reserved3 = 0x1;
		idt[index].reserved4 = 0x0;

		/* Set seg selector to Kernel CS */
		idt[index].seg_selector = KERNEL_CS;
		
		/* All vectors greater than 32 are initialized as interrupts */
		if(index >= INTER_VECT_START) 
		{
			idt[index].reserved3 = 0x0;
			SET_IDT_ENTRY(idt[index], general_interruption);
		}
		
		/* A syscall (int 0x80) comes from privilege level 3 */
		if(SYS_CALL == index)
		{
			idt[index].dpl = USER_PRIVILEGE;
		}
	}
	
	/* set exceptions */
	SET_IDT_ENTRY(idt[DIVIDE_BY_ZERO], divide_by_zero_error_exception);
	SET_IDT_ENTRY(idt[DEBUG], debug_exception);
	SET_IDT_ENTRY(idt[NON_MASK], non_maskable_interrupt_exception);
	SET_IDT_ENTRY(idt[BREAKPOINT], breakpoint_exception);
	SET_IDT_ENTRY(idt[OVERFLOW], overflow_exception);
	SET_IDT_ENTRY(idt[BOUND_RANGE], bound_range_exception);
	SET_IDT_ENTRY(idt[INVALID_OPCODE], invalid_opcode_exception);
	SET_IDT_ENTRY(idt[DEVICE_NOT_AVAIL], device_not_available_exception);
	SET_IDT_ENTRY(idt[DOUBLE_FAULT], double_fault_exception);
	SET_IDT_ENTRY(idt[SEGMENT_OVERRUN], segment_overrun_exception);
	SET_IDT_ENTRY(idt[INVALID_TSS], invalid_TSS_exception);
	SET_IDT_ENTRY(idt[NO_SEG], no_segment_exception);
	SET_IDT_ENTRY(idt[SEG_FAULT], seg_fault_exception);
	SET_IDT_ENTRY(idt[GENERAL_PROTECTION], general_protection_exception); 
	SET_IDT_ENTRY(idt[PAGE_FAULT], page_fault_exception);
	SET_IDT_ENTRY(idt[RESERVED], reserved_exception);
	SET_IDT_ENTRY(idt[FLOATING_POINT], floating_point_exception);
	SET_IDT_ENTRY(idt[ALIGN_CHECK], align_check_exception);
	SET_IDT_ENTRY(idt[MACHINE_CHECK], machine_check_exception);
	SET_IDT_ENTRY(idt[SIMD], SIMD_floating_point_exception);
	
	/*set interrupts*/
	SET_IDT_ENTRY(idt[INTER_VECT_START + RTC_IRQ_NUM], &rtc_linker);
	SET_IDT_ENTRY(idt[INTER_VECT_START + KBD_IRQ_NUM], &kbd_linker);
	SET_IDT_ENTRY(idt[SYS_CALL], &syscall_linker);
}


/* handle exceptions */
void divide_by_zero_error_exception(uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("divide_by_zero_error_exception!\n");
	while(1);
}

void debug_exception(uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("debug_exception!\n");
	while(1);
}

void non_maskable_interrupt_exception(uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("non_maskable_interrupt_exception!\n");
	while(1);
} 

void breakpoint_exception(uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("breakpoint_exception!\n");
	while(1);
}

void overflow_exception(uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("overflow_exception!\n");
	while(1);
}

void bound_range_exception(uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("bound_range_exception!\n");
	while(1);
}

void invalid_opcode_exception(uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("invalid_opcode_exception!\n");
	while(1);
}

void device_not_available_exception(uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("device_not_available_exception!\n");
	while(1);
}

void double_fault_exception(uint32_t error, uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("double_fault_exception!\n");
	while(1);
}

void segment_overrun_exception(uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("segment_overrun_exception!\n");
	while(1);
}

void invalid_TSS_exception(uint32_t error, uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("invalid_TSS_exception!\n");
	while(1);
}
void no_segment_exception(uint32_t error, uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("no_segment_exception!\n");
	while(1);
}

void seg_fault_exception(uint32_t error, uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("seg_fault_exception!\n");
	while(1);
}

void general_protection_exception(uint32_t error, uint32_t eip, uint32_t cs)
{	
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("general_protection_exception!\n");
	while(1);
}

void page_fault_exception (uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("page_fault_exception!\n");
	printf("Virtual address: %x  ", eip);

	uint32_t error;
	asm volatile ("mov %%cr2, %0"  \
	               : "=r" (error) \
				   :              \
				   : "memory");
	
	printf("Attempted access at: %x ", error);
	printf("Code segment: %x \n", cs);
	printf("The page directory entry for this address is %x ", get_pde(error));
	while(1);
}

void reserved_exception(uint32_t error, uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("reserved_exception!\n");
	while(1);
}

void floating_point_exception(uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("floating_point_exception!\n");
	while(1);
}

void align_check_exception(uint32_t error, uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("align_check_exception!\n");
	while(1);
}
void machine_check_exception(uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("machine_check_exception!\n");
	while(1);
}

void SIMD_floating_point_exception(uint32_t eip, uint32_t cs)
{
	cli();
	if(cs == USER_CS) {
		halt_by_exception();
	}
	clear();
	printf("SIMD_floating_point_exception!\n");
	while(1);
}
void general_interruption ()
{
	printf("general_interruption!\n");
}
