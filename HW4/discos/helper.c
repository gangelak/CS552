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
	size_t str1_len = strlen(string1);
	size_t str2_len = strlen(string2);

	if (str1_len != str2_len)
		return -1;
    for (int i = 0; i < str1_len; i++)
    {
	    if ( string1[i] != string2[i] )
		    return -1;
/*        if (string1[i] != string2[i])
        {
            return string1[i] < string2[i] ? -1 : 1;
        }

        if (string1[i] == '\0')
        {
            return 0;
        }
  */  }
	    return 0;
}
int strncmp(char string1[], char string2[], int size)
{
	for ( int i =0 ; i < size; i++ )
	{
		if (string1[i] != string2[i])
			return -1;
	}
	return 0;
}
//https://www.techiedelight.com/implement-strncpy-function-c/
void strncpy(char destination[], char source[], size_t num)
{
    // return if no memory is allocated to the destination
    if (destination == 0)
	            return ;

        for ( int i=0 ; i < num ; i++  )
	{
		            destination[i] = source[i];
			        
	}
	    destination[num] = '\0'; 

    // null terminate destination string
    // destination is returned by standard strncpy()
   // return ptr;
}




/*void memset(uint32_t *buf, uint32_t num, uint32_t size){*/
	/*uint32_t i;*/
	/*for (i =0; i < size; i++){*/
		/*buf[i] = num;*/
	/*}*/

	/*return;*/
/*}*/


