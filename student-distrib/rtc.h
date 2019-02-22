 /* rtc.h - Defines used in interactions with RTC.
  */
 
 #include "types.h"

 #ifndef _RTC_H
 #define _RTC_H
 
 /* IRQ number. */
 #define RTC_IRQ_NUM 0x8

 /* Port values. */
 #define RTC_PORT0 0x70
 #define RTC_PORT1 0x71
 
 /* RTC register indicies. */
 #define STAT_A_IDX 0xA
 #define STAT_B_IDX 0xB
 #define STAT_C_IDX 0xC
 
 /* Non-maskable interrupt disable bit. */
 #define NMI_DISABLE 0x80
 
 /* Inital rate value corresponding to 2 Hz. */
 #define RS_INIT 0xF
 
 /* Maximum rate allowed. */
 #define RTC_MAX_RATE 1024
 
 /* Minimum rate allowed. */
 #define RTC_MIN_RATE 2
 
 /* Masks for status register a. */
 #define A_UIP 0x80
 #define A_DV 0x70
 #define A_RS 0x0F
 
 /* Masks for status register b. */
 #define B_SET 0x80
 #define B_PIE 0x40
 #define B_AIE 0x20
 #define B_UIE 0x10
 #define B_SQWE 0x08
 #define B_DM 0x04
 #define B_24_12 0x02
 #define B_DSE 0x01

/* number of bytes that rate should be */
 #define NBYTES 4
 
 /* Functions */
 void rtc_open();
 
 int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
 
 int32_t rtc_write(int32_t fd, void* buf, uint32_t nbytes);
 
 int32_t rtc_close();
 
 extern void rtc_handler();

 #endif /* _RTC_H */
