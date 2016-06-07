#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "assembler.h"
#include "coder.h"

/* Code:  converts the command structure into binary code string and returns */
char *Code(CMD *cmd, char binStr[]){
	int i;
	for(i = 0; i < 18; i++)
		binStr[i] = '\0';

	switch(cmd->commandType){
	case A_COMMAND:
		binStr[0] = '0';
		if(cmd->symbol)
			strcpy(binStr+1, "111111111111111");
		else if(cmd->value < 32768 && cmd->value >= 0)
			strcpy(binStr+1, ItoB(cmd->value, binStr+1, 2, 15));
		else{
			printf("Error:  value outside allowable domain (0-32767) on line %d\n", nLine);
			exit(20);
		}
		break;
	case C_COMMAND:
		binStr[0] = binStr[1] = binStr[2] = '1';
		for(i = 0; i < sizeof(compTab)/sizeof(compTab[0]); i++){
			if(!strcmp(compTab[i][0], cmd->comp)){
				strcat(binStr, compTab[i][1]);
				break;
			}
			if(i + 1 == sizeof(compTab)/sizeof(compTab[0])){
				printf("Error:  unrecognized comp command \"%s on line %d\n", cmd->comp, nLine);
				exit(21);
			}
		}
		for(i = 0; i < sizeof(destTab)/sizeof(destTab[0]); i++){
			if(!strcmp(destTab[i][0], cmd->dest ? cmd->dest : "0")){
				strcat(binStr, destTab[i][1]);
				break;
			}
			if(i + 1 == sizeof(destTab)/sizeof(destTab[0])){
				printf("Error:  unrecognized dest command \"%s on line %d\n", cmd->dest, nLine);
				exit(22);
			}
		}
		for(i = 0; i < sizeof(jumpTab)/sizeof(jumpTab[0]); i++){
			if(!strcmp(jumpTab[i][0], cmd->jump ? cmd->jump : "0")){
				strcat(binStr, jumpTab[i][1]);
				break;
			}
			if(i + 1 == sizeof(jumpTab)/sizeof(jumpTab[0])){
				printf("Error:  unrecognized jump command \"%s on line %d\n", cmd->jump, nLine);
				exit(23);
			}
		}
		break;
	}
	binStr[16] = '\n';
	return binStr;
}

/* ItoB:  converts integer n(decimal) into a string number of base b padded with leading zeros to places */
char *ItoB(long n, char *s, int b, int places){

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
		places--;		
	}while(n /= b);
	if(sign < 0)
		s[i++] = '-';
	while(places-- > 0)
		s[i++] = '0';	
	s[i] = '\0';
	Reverse(s);

	return s;
}

/* Reverse:  reverses the input string */
void Reverse(char *s){
	int c, i, j;

	for(i = 0, j = strlen(s) - 1; i < j; i++, j--){
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}
