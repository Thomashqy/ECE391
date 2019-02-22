/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void
i8259_init(void)
{
  printf ("Entered i8259 initialization.\n");
  /* Send initialization command words to initialize device. */
  outb (ICW1, MASTER_PORT0);
  outb (ICW1, SLAVE_PORT0);
  
  outb (ICW2_MASTER, MASTER_PORT1);
  outb (ICW2_SLAVE, SLAVE_PORT1);
  
  outb (ICW3_MASTER, MASTER_PORT1);
  outb (ICW3_SLAVE, SLAVE_PORT1);
  
  outb (ICW4, MASTER_PORT1);
  outb (ICW4, SLAVE_PORT1);
  
  /* Mask all interrupts. */
  master_mask = MASTER_MASK_I;
  slave_mask = SLAVE_MASK_I;
  
  outb (master_mask, MASTER_PORT1);
  outb (slave_mask, SLAVE_PORT1);
}

/* Enable (unmask) the specified IRQ */
void
enable_irq(uint32_t irq_num)
{
  uint8_t mask;  /* Mask to enable IRQ on whichever PIC */
  
  /* Set mask to correspond to IRQ on PIC. */
  mask = 0x1 << (irq_num & IRQ_MASK);
  
  /* Determine which PIC the IRQ corresponds to based on bit
   * 3 of irq_num. */
  if (irq_num  >= NUM_PIC_IRQS) {
    slave_mask &= ~mask;
    outb (slave_mask, SLAVE_PORT1);
  }
  else {
    master_mask &= ~mask;
    outb (master_mask, MASTER_PORT1);
  }
}

/* Disable (mask) the specified IRQ */
void
disable_irq(uint32_t irq_num)
{
  uint8_t mask;  /* Mask to disable IRQ on whichever PIC */
  
  /* Set mask to correspond to IRQ on PIC. */
  mask = 0x1 << (irq_num & IRQ_MASK);
  
  /* Determine which PIC the IRQ corresponds to based on bit
   * 3 of irq_num. */
  if (irq_num > NUM_PIC_IRQS) {
    slave_mask |= mask;
    outb (slave_mask, SLAVE_PORT1);
  }
  else {
    master_mask |= mask;
    outb (master_mask, MASTER_PORT1);
  }
}

/* Send end-of-interrupt signal for the specified IRQ */
void
send_eoi(uint32_t irq_num)
{
	if (irq_num >= NUM_PIC_IRQS) {
		irq_num -= NUM_PIC_IRQS;
		outb (EOI | irq_num, SLAVE_PORT0);
		outb (EOI | SLAVE_IRQ_NUM, MASTER_PORT0);
	}
	else 
		outb (EOI | irq_num, MASTER_PORT0);
}

