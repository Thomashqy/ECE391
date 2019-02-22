 
 #include "types.h"

 #ifndef _TERMINAL_H
 #define _TERMINAL_H

/* IRQ number. */
#define KBD_IRQ_NUM 0x1

/* Port values. */
#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT 0x64

/* Bit signaling key was released. */
#define KBD_KEY_RELEASED 0x80

/* Terminal buffer size. */
#define TERMINAL_BUFF_SIZE 128

/* Scan table size. */
#define SCAN_TABLE_SIZE 128

/* Scan codes. */
#define SC_1 2
#define SC_2 3
#define SC_3 4
#define SC_4 5
#define SC_5 6
#define SC_Q 16
#define SC_P 25
#define SC_A 30
#define SC_L 38
#define SC_Z 44
#define SC_M 50
#define SC_L 38

#define SC_CTRL_PRESS 29
#define SC_CTRL_RELEASE 157
#define SC_SHIFT_L_PRESS 42
#define SC_SHIFT_L_RELEASE 170
#define SC_SHIFT_R_PRESS 54
#define SC_SHIFT_R_RELEASE 182
#define SC_ALT_PRESS 56
#define SC_ALT_RELEASE 184
#define SC_CAPS_PRESS 58
#define SC_CAPS_RELEASE 186
#define SC_ESC 1

void init_terminal();

/* System Calls prototypes */

int32_t terminal_open (uint32_t pid);
int32_t terminal_read(int32_t fd, unsigned char* buffer, int32_t nbytes);
int32_t terminal_write(int32_t fd, unsigned char* buffer, int32_t nbytes);
int32_t terminal_close ();

/* Functions */
extern void kbd_handler();
int is_letter(uint8_t scancode_test);

/*
int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd); 
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);
*/

 #endif /* _TERMINAL_H */
