#include "types.h"
#ifndef _HELPER
#define _HELPER

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

void memset(uint32_t *buf, uint32_t num, uint32_t size);

#endif
