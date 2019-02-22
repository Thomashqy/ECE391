
#include "terminal.h"
#include "lib.h"
#include "i8259.h"
#include "pcb.h"
#include "act_syscall.h"

/* scan code vars */
int alt = 0;
int caps_active = 0;
int caps_held = 0;
int shift = 0;
int ctrl = 0;
uint8_t scancode;

int x = 0;
int y = 0;

unsigned char terminal_buffers[MAX_NUM_TASKS][TERMINAL_BUFF_SIZE];
uint8_t t_buffer_starts[MAX_NUM_TASKS];
uint8_t t_buffer_ends[MAX_NUM_TASKS];

volatile uint8_t enter_pressed = 0;

/*
  terminal_open
  Desc: opens the terminal for a process
  Inputs: pid - The id of the process
  Return: returns 0 on success
  Side Effects: Updates VGA cursor
*/
int32_t terminal_open(uint32_t pid)
{
  //clear();
  
  t_buffer_starts[pid] = 0;
  t_buffer_ends[pid] = 0;
  enter_pressed = 0;
  update_cursor(0, 0);
  memset(terminal_buffers[pid], 0, TERMINAL_BUFF_SIZE);
  enable_irq(KBD_IRQ_NUM);
  return 0;
}

/*
  terminal_read
  Desc: Copies keyboard input into buffer.
  Inputs: fd - the file descriptor of the process, buffer - the buffer to copy keyboard input into,
          nbytes - the size of buffer
  Return: returns the number of bytes read
  Side Effects: Blocks until enter is pressed
*/
int32_t terminal_read(int32_t fd, unsigned char* buffer, int32_t nbytes)
{
  int32_t o_buff_idx = 0;  /* Index into the output buffer */

  /* Check for valid inputs. */
  if((buffer == NULL) || (nbytes < 0))
    return -1;

  /* Get pid for this task. */
   uint32_t pid = get_pcb()->pid;

  sti();
  /* Wait for command to be entered. */
  while(enter_pressed == 0) {
    continue;
  }
  
  /* Copy buffer data into the output buffer. */
  while((o_buff_idx < nbytes)&&(t_buffer_starts[pid] < t_buffer_ends[pid])) {
    buffer[o_buff_idx] = terminal_buffers[pid][t_buffer_starts[pid]];
	
	o_buff_idx++;
	t_buffer_starts[pid]++;
  }
  
  /* If entire command was read, clear buffer and enter_pressed flag. */
  if(t_buffer_starts[pid] == t_buffer_ends[pid]) {
    t_buffer_starts[pid] = 0;
	t_buffer_ends[pid] = 0;
	enter_pressed = 0;
  }

  cli();
  return o_buff_idx;
}

/*
  terminal_write
  Desc: Writes data from buffer to video memory
  Inputs: fd - the file descriptor of the process, buffer - the buffer to copy data from,
          nbytes - the size of buffer
  Return: returns 0 on success, -1 on failure
  Side Effects: Writes to video memory and updates VGA cursor
*/
int32_t terminal_write(int32_t fd, unsigned char* buffer, int32_t nbytes)
{
  int32_t i;
  int w_x, w_y;
	
  if(buffer == NULL)
    return -1;
  
  /* Prevent keyboard from interrupting. */
  cli();
  for(i = 0; i < nbytes; i++)
    putc_with_scrolling(buffer[i]);
  sti();
	
  get_screen_xy(&w_x, &w_y);
  update_cursor(w_y, w_x);
  return 0;
}

int32_t terminal_close ()
{
  return 0;
}

void
print_cur_buffer(uint32_t pid)
{
	uint8_t i;
	for(i = 0; i < t_buffer_ends[pid]; i++)
		putc_with_scrolling(terminal_buffers[pid][i]);
	get_screen_xy(&x, &y);
    update_cursor(y, x);
}


 /* Scancode table from http://www.osdever.net/bkerndev/Docs/keyboard.htm */
 unsigned char scan_table[SCAN_TABLE_SIZE] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};
/* modified Scancode table from above to display shifted chars*/
unsigned char scan_table_shift[SCAN_TABLE_SIZE] =
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', /* 9 */
  '(', ')', '_', '+', '\b', /* Backspace */
  '\t',     /* Tab */
  'Q', 'W', 'E', 'R', /* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter key */
    0,      /* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
 '"', '~',   0,    /* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',      /* 49 */
  'M', '<', '>', '?',   0,        /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

 /*
  * is_letter(uint8_t scancode_test)
  * Description:
  * return 1 if keypressed is
  * a letter
  */
  int is_letter(uint8_t scancode_test)
  {
    if(( scancode_test >= SC_Q && scancode_test <= SC_P ) || ( scancode_test >= SC_A && scancode_test <= SC_L ) || ( scancode_test >= SC_Z && scancode_test <= SC_M ))
      return 1;
    else
      return 0;
  }

 /*
  * kbd_handler()
  *
  * Description:
  * Test handler for keyboard.
  */

 extern void
 kbd_handler ()
 {
	uint8_t status;
  uint32_t pid = get_pcb()->pid;
	
	cli();
	status = inb(KBD_STATUS_PORT);
	//send_eoi (KBD_IRQ_NUM);
	if(status & 0x1)
	  scancode = inb (KBD_DATA_PORT);
	else {
	  send_eoi (KBD_IRQ_NUM);
	  sti();
	  return;
	}
	//sti();
  
  /* uncomment line below to display scancode */
  //printf("%d", scancode);

  /* ctrl commands */
  if((ctrl == 1) && !alt) {
	  switch (scancode) {
		  /* Clear screen. */
		  case SC_L: clear();
					 update_cursor(0, 0);
					 print_cur_buffer(pid);
					 break;
		  
		  /* Halt current task. */
	  }
  }

  /* ctrl key handler */
  if(scancode == SC_CTRL_PRESS)
    ctrl = 1;
  if(scancode == SC_CTRL_RELEASE)
    ctrl = 0;

  /* shift key handler */
  if(scancode == SC_SHIFT_L_PRESS || scancode == SC_SHIFT_R_PRESS)
    shift = 1;
  if(scancode  == SC_SHIFT_L_RELEASE || scancode  == SC_SHIFT_R_RELEASE)
    shift = 0;

  /* alt key handler */
  if(scancode == SC_ALT_PRESS)  
    alt = 1;
  if (scancode  == SC_ALT_RELEASE)
    alt = 0;

  /* caps lock handler */
  if(scancode == SC_CAPS_PRESS && !caps_held)
  {
    caps_active = !caps_active;/* do nothing */
  }
  if(scancode == SC_CAPS_RELEASE)
    caps_held = 0;

  /* key presses only, releases do nothing */
  if(!(scancode & KBD_KEY_RELEASED) && !alt && !ctrl && !enter_pressed)
  {
    /* Print only printable character */
    if((scan_table[scancode] != 0) && (scancode != SC_ESC) && (scan_table[scancode] != '\b') && (scan_table[scancode] != '\t'))
    {
      if(t_buffer_ends[pid] < (TERMINAL_BUFF_SIZE-1) && scan_table[scancode] != '\n')
      {
	      if(is_letter(scancode)) /* letter */
	      {
	        if(caps_active != shift)
	        {
	          putc_with_scrolling(scan_table_shift[scancode]);
	          terminal_buffers[pid][t_buffer_ends[pid]] = scan_table_shift[scancode];
	          t_buffer_ends[pid]++;
	        }
	        else
	        {
	          putc_with_scrolling(scan_table[scancode]);
	          terminal_buffers[pid][t_buffer_ends[pid]] = scan_table[scancode];
	          t_buffer_ends[pid]++;
	        }
	      }
	      else/* number */
	      {
	        if(shift)
	        {
	          putc_with_scrolling(scan_table_shift[scancode]);
	          terminal_buffers[pid][t_buffer_ends[pid]] = scan_table_shift[scancode];
	          t_buffer_ends[pid]++;
	        }
	        else
	        {
	          putc_with_scrolling(scan_table[scancode]);
	          terminal_buffers[pid][t_buffer_ends[pid]] = scan_table[scancode];
	          t_buffer_ends[pid]++;
	        }
	      }
  	  }

      /* enter key handler */
      if(scan_table[scancode] == '\n')
      {
		    terminal_buffers[pid][t_buffer_ends[pid]] = '\n';
		    t_buffer_ends[pid]++;
      	putc_with_scrolling('\n');
	      enter_pressed = 1;
      }
    }
    else if(scan_table[scancode] == '\b')
    {
    	if(t_buffer_ends[pid] > t_buffer_starts[pid])
    	{
    		t_buffer_ends[pid]--;
    		backspace();
    	}
    }

    /* update screen */
    get_screen_xy(&x, &y);
    update_cursor(y, x);
  }
  send_eoi (KBD_IRQ_NUM);
  sti();
 }

