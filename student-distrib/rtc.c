 /* rtc.c - Functions to interact with RTC
  */
  
 #include "rtc.h"
 #include "lib.h"
 #include "i8259.h"
 #include "terminal.h"
 
 /* Flag for determining if RTC interrupt occurred. */
 volatile int rtc_interrupt_occurred = 0;
 
 /*
  * rtc_open()
  *
  * Description:
  * Initializes the RTC.
  */
  void
  rtc_open()
  {
	uint8_t prev;
	
	/* Get status register b. */
	outb (NMI_DISABLE | STAT_B_IDX, RTC_PORT0);
    prev = inb (RTC_PORT1);
    
    /* Enable interrupts on RTC. */
    outb (NMI_DISABLE | STAT_B_IDX, RTC_PORT0);
    outb (prev | B_PIE, RTC_PORT1);
	
	/* Get status register a. */
	outb (NMI_DISABLE | STAT_A_IDX, RTC_PORT0);
    prev = inb (RTC_PORT1);
	
	/* Set rate. */    
    outb (NMI_DISABLE | STAT_A_IDX, RTC_PORT0);
    outb ((prev & ~A_RS) | RS_INIT, RTC_PORT1);
	
    /* Enable IRQ on PIC. */
    enable_irq (RTC_IRQ_NUM);
  }
  
/*
 * rtc_read
 *   DESCRIPTION: Blocks until an RTC interrupt occurs.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: enables interrupts
 */
  int32_t
  rtc_read(int32_t fd, void* buf, int32_t nbytes)
  {	
	  /* Set interrupt flag. */
	   rtc_interrupt_occurred = 0;
	  sti();
	  /* Wait for handler to clear flag. */
	  while (rtc_interrupt_occurred == 0) {
		  continue;
	  }
	  cli();
	
	  return 0;
  }
  
/*
 * rtc_write
 *   DESCRIPTION: Sets the RTC interrupt rate.
 *   INPUTS: rate - The rate to set on the RTC
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Changes the rate of the RTC.
 */
  int32_t
  rtc_write(int32_t fd, void* buf, uint32_t nbytes)
  {

  	  if(nbytes != NBYTES)
  		return -1;

  	  uint32_t rate = *((uint32_t*)buf);

	  uint8_t rs = RS_INIT;  /* The rs value that will be written to RTC */
	  uint8_t prev_a;        /* Value of RTC status register a */
	  uint32_t i;            /* Variable for loop iteration */
	  
	  /* Check if rate is within the valid range. */
	  if((rate > RTC_MAX_RATE)||(rate <= 1))
		  return -1;
	  
	  /* Check if rate is a power of two. */
	  if(!((rate & (~rate + 1)) == rate))
		  return -1;
	  
	  /* Determine what value to write to RTC based on RTC rate table. */
	  for(i = rate >> 1; !(i & 0x1); i >>= 1) {
		  rs--;
	  }
	  
	  /* Clear interrupts to write to RTC. Critical section. */
	  cli();
	  
	  /* Get status register a. */
	  outb (NMI_DISABLE | STAT_A_IDX, RTC_PORT0);
      prev_a = inb (RTC_PORT1);
	
	  /* Set rate. */    
      outb (NMI_DISABLE | STAT_A_IDX, RTC_PORT0);
      outb ((prev_a & ~A_RS) | rs, RTC_PORT1);
	  
	  /* Critical section ends. */
	  sti();
	  
	  return nbytes;
  } 
  
/*
 * rtc_close
 *   DESCRIPTION: Closes the RTC.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
  int32_t
  rtc_close()
  {
	  return 0;
  }

  
 /*
  * rtc_handler
  *
  * Description:
  * Handler for the rtc.
  */
  extern void
  rtc_handler()
  {
	cli();
	outb (STAT_C_IDX, RTC_PORT0); // Select register C to allow further interrupts.
	inb (RTC_PORT1);
	
	/* Set interrupt flag to signal that interrupt occurred. */
	rtc_interrupt_occurred = 1;
	
	send_eoi (RTC_IRQ_NUM);
	sti();
  }
  
