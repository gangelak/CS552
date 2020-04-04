#ifndef _HELPER
#define _HELPER

#include "types.h"

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

void memset(uint32_t *, uint32_t, uint32_t );

//inline void outb( unsigned char , unsigned short);
//inline unsigned char inb( unsigned short);
//inline void io_wait(void);


inline unsigned char inb( unsigned short usPort  ) {

	    unsigned char uch;

	        asm volatile( "inb %1,%0" : "=a" (uch) : "Nd" (usPort)  );
		    return uch;

}

inline void outb( unsigned char uch, unsigned short usPort  ) {
	
	    asm volatile( "outb %0,%1" : : "a" (uch), "Nd" (usPort)  );

}

inline void io_wait(void)
{
	    /* Port 0x80 is used for 'checkpoints' during POST. */
	    /* The Linux kernel seems to think it is free for use :-/ */
	    asm volatile ( "outb %%al, $0x80" : : "a"(0)  );
	        /* %%al instead of %0 makes no difference.  TODO: does the register need to be zeroed? */

}

#endif
