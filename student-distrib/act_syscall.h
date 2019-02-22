/* System call header file. */
#ifndef ACT_SYSCALL_H
#define ACT_SYSCALL_H

#include "types.h"

#define FILE_HEADER_SIZE 40
#define USER_VIRT_PAGE 0x08000000
#define PROG_PAGE_SIZE 0x400000
#define MAX_NUM_TASKS 6

#define USER_PHYS_START 0x00800000
#define KERNEL_STACK_OFFSET 0x4
#define KERNEL_STACK_SIZE 0x00002000
#define USER_PAGE_FLAGS 0x097
#define USER_COPY_START 0x08048000
#define EXE_MAGIC_NUM_SIZE 4

#define SHIFT_1_BYTE 8
#define SHIFT_2_BYTE 16
#define SHIFT_3_BYTE 24

extern int32_t halt(uint8_t status);
extern int32_t execute(const uint8_t* command);
extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, void* buf, int32_t nbytes);
extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);
extern int32_t getargs(uint8_t* buf, int32_t nbytes);
extern int32_t vidmap(uint8_t** screen_start);
extern int32_t set_handler(int32_t signum, void* handler_address);
extern int32_t sigreturn(void);

int check_executable(uint8_t* header);
int32_t halt_by_exception();
int32_t error_func();
uint32_t read_virt_addr(uint8_t* header);

#endif

