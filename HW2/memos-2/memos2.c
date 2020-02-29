#include "multiboot.h"

#define TEXT_BUFFER_LOC 0xB8000
#define COLS 80
#define ROWS 24

#define COLOR 0x0F << 8

unsigned short *text_buffer = (unsigned short*)TEXT_BUFFER_LOC;

typedef struct multiboot_memory_map {
	unsigned int size;
	unsigned int base_low, base_high;
	unsigned int len_low, len_high;
	unsigned int type;
} multiboot_memmap_t;

typedef multiboot_memmap_t mmap_entry_t;
struct cursor {
	int x, y;
} csr;

// refer to geeksforgeeks.org for implementation of iota()
/*void my_itoa ( int num, char* str, int base)*/
/*{*/
	/*int i = 0 ;*/

	/*// handle 0 explicitely*/
	/*if ( num ==0 )*/
	/*{*/
		/*str[i++] = '0';*/
		/*str[i] = '\0';*/

	/*}*/
	/*// process individual digits */
	/*while (num != 0) */
	/*{ */
		/*int rem = num % base; */
		/*str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0'; */
		/*num = num/base; */
	/*}*/

	/*str[i] = '\0';*/
	/*// now we have to reverse the char**/
	/*i--;*/
	/*char tmp = "\0";*/
	/*for ( int j = 0 ; i > j; j++, i-- )*/
	/*{*/
		/*tmp = str[i];*/
		/*str[i] = str[j];*/
		/*str[j] = tmp;*/
	/*}*/
	
/*}*/


char * itoa( int value, char * str, int base )
{
    char * rc;
    char * ptr;
    char * low;
    // Check for supported base.
    if ( base < 2 || base > 36 )
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    // Set '-' for negative decimals.
    if ( value < 0 && base == 10 )
    {
        *ptr++ = '-';
    }
    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do
    {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while ( value );
    // Terminating the string.
    *ptr-- = '\0';
    // Invert the numbers.
    while ( low < ptr )
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
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


void kmain (multiboot_info_t* mbt, unsigned long magic) {
	print("\n\n\n\n\n\n\n\nMemOS: Welcome *** System Memory is:\n");
	char *str;
	
	// check whether the data is valid or not
	if (mbt->flags & 0b1000000)
	{
		mmap_entry_t* ent = mbt->mmap_addr;
		while( ent < mbt->mmap_addr + mbt->mmap_length && (ent->base_low + ent->len_low != 0))
		{
			// Address Range: [ 0x00000000 0x0000000 ] status: 
			my_memset(str);
			print("Address Range: [");
			print(" 0x");
			str = itoa(ent->base_low, str, 16);
			print(str);

			my_memset(str);
			print(" 0x");
			str = itoa(ent->base_low + ent->len_low, str, 16);
			print(str);
			print(" ]");
			print(" status: ");
			str = itoa(ent->type, str, 10);
			print(str);
			print("\n");
			ent = (mmap_entry_t*) ((unsigned int) ent + ent->size + sizeof(ent->size));
		}
	}
	for (;;) {}
}
