#include "multiboot.h"

#define TEXT_BUFFER_LOC 0xB8000
#define COLS 80
#define ROWS 24

#define COLOR 0x0F << 8

unsigned short *text_buffer = (unsigned short*)TEXT_BUFFER_LOC;

struct cursor {
	int x, y;
} csr;

void put(unsigned char c)
{
	unsigned short *pos;
	if(c == 0x09)
	{
		csr.x = (csr.x + 8 ) & ~( 8 - 1);
	}
	else if ( c == '\r' )
	{
		csr.x = 0;
	}
	else if (c == '\n')
	{
		csr.x = 0;
		csr.y++;
	}
	else if (c >= ' ')
	{
		pos = text_buffer + (csr.y*COLS) + csr.x ;
		*pos = COLOR | c;
		csr.x++;
	}
}

void print(char *text) {

	int i;
	for (i =0; text[i] != '\0'; i ++)
	{
		put(text[i]);
	}
}


void kmain (multiboot_info_t* mbd, unsigned long magic) {
	print("MemOS: Welcome *** System Memory is:\n");
	for (;;) {}
}
