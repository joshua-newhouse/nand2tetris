#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "jlib.h"

void Reverse(char s[]);

/* ItoB:  converts base b (where 2 <= b <= 16) integer n to a string s */
char *jlib_ItoB(long n, char s[], int b){

	if(n > INT_MAX || n < INT_MIN){
		printf("Number %ld is outside the integer range of this machine. (%d to %d)\n", n, INT_MIN, INT_MAX);
		s[0] = '\0';
		exit(1);
	}
	if(b < 2 || b > 16){
		printf("Base (%d) must be greater than or equal to 2 and less than or equal to 16.\n", b);
		s[0] = '\0';
		exit(2);
	}

	int i = 0, sign = 1, a;
	char c;

	if(n < 0)
		sign = -1;
	do{
		a = sign * (n % b);
		if(a > 9)
			s[i++] = a - 10 + 'A';
		else
			s[i++] = a + '0';		
	}while(n /= b);
	if(sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	Reverse(s);

	return s;
}

/* Reverse:  reverses string s */
void Reverse(char s[]){
	int c, i, j;

	for(i = 0, j = strlen(s) - 1; i < j; i++, j--){
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}
