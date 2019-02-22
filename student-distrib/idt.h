#ifndef IDT_H
#define IDT_H

#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"

#define SYS_CALL 0x80
#define KERNEL_PRIVILEGE 0
#define USER_PRIVILEGE 3
#define INTER_VECT_START 32

/*Exception Vectors*/
#define DIVIDE_BY_ZERO 0
#define DEBUG 1
#define NON_MASK 2
#define BREAKPOINT 3
#define OVERFLOW 4
#define BOUND_RANGE 5
#define INVALID_OPCODE 6
#define DEVICE_NOT_AVAIL 7
#define DOUBLE_FAULT 8
#define SEGMENT_OVERRUN 9
#define INVALID_TSS 10
#define NO_SEG 11
#define SEG_FAULT 12
#define GENERAL_PROTECTION 13
#define PAGE_FAULT 14
#define RESERVED 15
#define FLOATING_POINT 16
#define ALIGN_CHECK 17
#define MACHINE_CHECK 18 
#define SIMD 19

void init_idt ();
void divide_by_zero_error_exception(uint32_t eip, uint32_t cs);
void debug_exception(uint32_t eip, uint32_t cs);
void non_maskable_interrupt_exception(uint32_t eip, uint32_t cs);
void breakpoint_exception(uint32_t eip, uint32_t cs);
void overflow_exception(uint32_t eip, uint32_t cs);
void bound_range_exception(uint32_t eip, uint32_t cs);
void invalid_opcode_exception(uint32_t eip, uint32_t cs);
void device_not_available_exception (uint32_t eip, uint32_t cs);
void double_fault_exception (uint32_t error, uint32_t eip, uint32_t cs);
void segment_overrun_exception (uint32_t eip, uint32_t cs);
void invalid_TSS_exception (uint32_t error, uint32_t eip, uint32_t cs);
void no_segment_exception (uint32_t error, uint32_t eip, uint32_t cs);
void seg_fault_exception (uint32_t error, uint32_t eip, uint32_t cs);
void general_protection_exception (uint32_t error, uint32_t eip, uint32_t cs);
void page_fault_exception (uint32_t eip, uint32_t cs);
void reserved_exception (uint32_t error, uint32_t eip, uint32_t cs);
void floating_point_exception (uint32_t eip, uint32_t cs);
void align_check_exception (uint32_t error, uint32_t eip, uint32_t cs);
void machine_check_exception (uint32_t eip, uint32_t cs);
void SIMD_floating_point_exception (uint32_t eip, uint32_t cs);
void general_interruption ();


#endif /* IDT_H */
