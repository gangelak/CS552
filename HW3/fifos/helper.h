#include "vga.h"


#define TEXT_BUFFER_LOC 0xB8000
#define COLS 80
#define ROWS 24
#define MAX_INT 0xFFFFFFFF

#define COLOR 0x0F << 8


#define nop() asm("nop")


struct cursor {
	int x, y;
} csr;


unsigned short *text_buffer = (unsigned short*)TEXT_BUFFER_LOC;

typedef struct multiboot_memory_map {
	unsigned int size;
	unsigned int base_low, base_high;
	unsigned int len_low, len_high;
	unsigned int type;
} multiboot_memmap_t;

typedef multiboot_memmap_t mmap_entry_t;


/* Convert the integer D to a string and save the string in BUF. If
 *    BASE is equal to 'd', interpret that D is decimal, and if BASE is
 *       equal to 'x', interpret that D is hexadecimal. */
void itoa (char *buf, int base, int d)
{
	char *p = buf;
	char *p1, *p2;
	unsigned long ud = d;
	int divisor = 10;

	/* If %d is specified and D is minus, put `-' in the head. */
	if (base == 'd' && d < 0)
	{
		*p++ = '-';
		buf++;
		ud = -d;
	}
	else if (base == 'x')
		divisor = 16;
		    
	/* Divide UD by DIVISOR until UD == 0. */
	do
	{
		int remainder = ud % divisor;
		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
	}
	while (ud /= divisor);
		      
	/* Terminate BUF. */
	*p = 0;
		      
	/* Reverse BUF. */
	p1 = buf;
	p2 = p - 1;
	while (p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}
}


void msleep(int time){
	for( int i = 0 ; i < time ; i++) {
		nop();
	}
}

void pad(char* str)
{
 char padded[] = "00000000\n";
 int i = 0;
 for (i; str[i] != '\0'; i ++) {}

 int j = 8 - i;
 int k = 0;
 for ( j ; j < 9 ; j++ )
 {
	 padded[j] = str[k];
	 k++;
 }
 print_s(padded);
 str = padded;
}



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


void my_memset(char *str){
	int i;
	for (i =0; i< 50; i++){
		str[i] = '\0';
	}

	return;
}

