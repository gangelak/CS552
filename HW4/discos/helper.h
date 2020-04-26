#ifndef _HELPER
#define _HELPER

#include "types.h"

static inline void outb( uint16_t, uint8_t);
static inline unsigned char inb( uint16_t);
static inline void io_wait(void);

#define nop() asm("nop")
typedef struct multiboot_memory_map {
	unsigned int size;
	unsigned int base_low, base_high;
	unsigned int len_low, len_high;
	unsigned int type;
} multiboot_memmap_t;

typedef multiboot_memmap_t mmap_entry_t;


void itoa (char *, int, int );

void msleep(int );
void put(unsigned char);

void print(char *);

void memset(void *s, int c, size_t n);


//static inline void outb( unsigned char, unsigned short);
//static inline unsigned char inb( unsigned short);
//static inline void io_wait(void);

static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

inline void io_wait(void)
{
     //Port 0x80 is used for 'checkpoints' during POST. 
     //The Linux kernel seems to think it is free for use :-/ 
    asm volatile ( "outb %%al, $0x80" : : "a"(0) );
 //    %%al instead of %0 makes no difference.  TODO: does the register need to be zeroed? 
}

#endif
