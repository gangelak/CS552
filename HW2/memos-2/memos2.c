#include "multiboot.h"

#define TEXT_BUFFER_LOC 0xB8000
#define COLS 80
#define ROWS 24
#define MAX_INT 0xFFFFFFFF

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


char * itoa( unsigned long value, char * str, int base )
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
        *ptr++ = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[35 + value % base];
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
 print(padded);
 str = padded;
}
void my_memset(char *str){
	int i;
	for (i =0; i< 50; i++){
		str[i] = '\0';
	}

	return;
}


void kmain (multiboot_info_t* mbt, unsigned long magic) {
	print("\n\n\n\n\n\n\n\n");
	char* str;
	unsigned long temp;
	unsigned long total_len = 0;
	
	// check whether the data is valid or not
	if (mbt->flags & 0b1000000)
	{
		mmap_entry_t* ent = mbt->mmap_addr;
		int flag = 0;
		while( ent < mbt->mmap_addr + mbt->mmap_length && flag == 0)
		{
			// Address Range: [ 0x00000000 0x0000000 ] status: 
			my_memset(str);
			print("Address Range: [ ");

			// first 32 bit
			str = itoa(ent->base_high, str, 16);
			pad(str);
//			print(str);
			my_memset(str);
			
			// second 32 bit of addr
			str = itoa(ent->base_low, str, 16);
			pad(str);
			my_memset(str);

			// printing base_addr + len

			print(":");

			if ((unsigned int)(ent->base_low + ent->len_low) == 0 )
			{
				str = itoa( (ent->len_high+ent->base_high +1) , str ,16);
				pad(str);
				my_memset(str);
				flag = 1;

			}
			else
			{
				str = itoa(ent->len_high+ent->base_high, str ,16);
				pad(str);
				my_memset(str);	
			}
			
			str = itoa(ent->len_low + ent->base_low, str ,16);
			pad(str);
			print("]");
			print(" status: ");
			str = itoa(ent->type, str, 10);
			print(str);
			print("\n");
			ent = (mmap_entry_t*) ((unsigned int) ent + ent->size + sizeof(ent->size));

			total_len += ent->len_low;

		}
			print("MemOS: Welcome *** System Memory is: ");
			my_memset(str);
			temp = ((((total_len) >> 20) &0xfff) + 1);
			str = itoa(temp, str, 10);
			print(str);
			print("MB");
			my_memset(str);
			print("\n");
	}
	while(1){;}
}
