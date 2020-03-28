#include "vga.h"
#include "helper.h"

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


void my_memset(char *str){
	int i;
	for (i =0; i< 50; i++){
		str[i] = '\0';
	}

	return;
}

