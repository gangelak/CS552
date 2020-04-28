#include "vga.h"
#include "helper.h"
#include "types.h"

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

 int j;
 int k = 0;
 for ( j= 8 - i ; j < 9 ; j++ )
 {
	 padded[j] = str[k];
	 k++;
 }
 print_s(padded);
 str = padded;
}

void memset(void *s, int c, size_t n)
{
    unsigned char* p=s;
    while(n--)
        *p++ = (unsigned char)c;
   // return s;
}

int strcmp(char string1[], char string2[] )
{
    for (int i = 0; ; i++)
    {
        if (string1[i] != string2[i])
        {
            return string1[i] < string2[i] ? -1 : 1;
        }

        if (string1[i] == '\0')
        {
            return 0;
        }
    }
}

//https://www.techiedelight.com/implement-strncpy-function-c/
char* strncpy(char* destination, const char* source, size_t num)
{
    // return if no memory is allocated to the destination
    if (destination == 0)
        return 0;
 
    // take a pointer pointing to the beginning of destination string
    char* ptr = destination;
 
    // copy first num characters of C-string pointed by source
    // into the array pointed by destination
    while (*source && num--)
    {
        *destination = *source;
        destination++;
        source++;
    }
 
    // null terminate destination string
    *destination = '\0';
 
    // destination is returned by standard strncpy()
    return ptr;
}




/*void memset(uint32_t *buf, uint32_t num, uint32_t size){*/
	/*uint32_t i;*/
	/*for (i =0; i < size; i++){*/
		/*buf[i] = num;*/
	/*}*/

	/*return;*/
/*}*/


