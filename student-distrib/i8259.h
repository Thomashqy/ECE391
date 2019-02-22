/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_PORT0 0x20
#define MASTER_PORT1 0x21
#define SLAVE_PORT0  0xA0
#define SLAVE_PORT1  0xA1

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1    0x11
#define ICW2_MASTER   0x20
#define ICW2_SLAVE    0x28
#define ICW3_MASTER   0x04
#define ICW3_SLAVE    0x02
#define ICW4          0x01

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI             0x60

/* Initial mask values for master and slave PICs */
#define MASTER_MASK_I 0xFB
#define SLAVE_MASK_I 0xFF

/* IRQ for slave PIC on master PIC. */
#define SLAVE_IRQ_NUM 0x2

/* Mask for determining IRQ number relative to one PIC. */
#define IRQ_MASK 0x7

/* Number of IRQs on one PIC. */
#define NUM_PIC_IRQS 0x8

/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */
