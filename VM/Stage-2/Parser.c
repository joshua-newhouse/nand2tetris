#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Parser.h"
#include "CmdTables.h"

#define DEBUG(expr) printf("Line " #expr "\n")

/* skipWhiteSpace:  skips leading white space in s */
#define skipWhiteSpace(s) {while((*s == ' ' || *s == '\t' || *s == '\0') && strPos--) s++;}

/* markStrEnd:  puts '\0' at the end of each word in the string s */
#define markStrEnd(s)  {int i = 0; \
			while(*(s+i) != ' ' && *(s+i) != '\t' && *(s+i) != '\n' && *(s+i) != '\0'){\
				i++; \
			}\
			*(s+i) = '\0';}

/* moveToNextString:  moves sting pointer s to the next word in string s */
#define moveToNextStr(s) {while(*s++ && strPos--);	\
				strPos--;}

/* errCMD:  prints error message and terminates program */
#define errCMD(string, n, e) {printf("Error:  unknown command %s on line %d\n", string, n);	\
			     exit(e);}

extern int nLine;

int isArith(char *s);
int isInteger(char *s);

/* Parser_Constructor:  initializes Parser_command structure to 0 */
void Parser_Constructor(Parser_command *cmd){
	cmd->commandType = 0;
	cmd->arg1 = NULL;
	cmd->arg2 = NULL;
}

/* Parser_advance: parses the current line and stores the info in the Parser_command structure */
int Parser_advance(char *s, Parser_command *cmd, int *fIndex){
	int strPos = strlen(s);
	while(strPos > 0){
		switch(*s){
		case '\n':			/* Skip blank lines */
			return 0;
		case ' ':			/* Skip white space */
		case '\t':
		case '\0':
			skipWhiteSpace(s);
			break;
		case '/':			/* Ignore comments */
			if(*++s == '/')
				return 0;
			else
				errCMD(--s, nLine, 30);
		case '-':			/* Negative numbers not allowed */
			errCMD(s, nLine, 30);
		case '0': case '1': case '2': case '3': case '4':	/* Captures trailing number for certain commands */
		case '5': case '6': case '7': case '8': case '9':
			markStrEnd(s);
			if((cmd->commandType == C_PUSH || cmd->commandType == C_POP ||
			   cmd->commandType == C_FUNCTION || cmd->commandType == C_CALL) && isInteger(s)){
				cmd->arg2 = s;
				return 1;
			}
			else
				errCMD(s, nLine, 31);
		default:
			markStrEnd(s);
			int i;
			for(i = 0; stackOpsTab[i] && !cmd->arg1; i++){
				if(!strcmp(s, stackOpsTab[i])){		/* C_PUSH or C_POP */
					cmd->commandType = i + 1;
					moveToNextStr(s);
					skipWhiteSpace(s);
					markStrEnd(s);
					break;
				}
			}
			if(!(cmd->commandType == C_PUSH || cmd->commandType == C_POP || cmd->commandType == C_RETURN)){
				if((i = isArith(s)) && !cmd->arg1){	/* C_ARITHMETIC and assigns operation to arg 1 */
					cmd->commandType = C_ARITHMETIC;
					cmd->arg1 = arithTab[i-1];
					moveToNextStr(s);
				}
				else
					errCMD(s, nLine, 32);
				return 1;
			}
			for(i = 0; memSegTab[i] && (cmd->commandType == C_PUSH || cmd->commandType == C_POP)
				   && cmd->commandType != C_RETURN; i++)
				if(!strcmp(s, memSegTab[i])){		/* assigns memory segment to arg 1 */
					cmd->arg1 = memSegTab[i];
					moveToNextStr(s);
					break;
				}
			if(!cmd->arg1)
				errCMD(s, nLine, 33);
			break;
		}
	}
}

/* isArith:  returns index plus 1 if s is an arithmetic command found in arithTab, 0 otherwise */
int isArith(char *s){
	int i;
	for(i = 0; arithTab[i]; i++){
		if(!strcmp(s, arithTab[i]))
			return i + 1;
	}
	return 0;
}

/* isInterger:  returns 1 if string s is a base 10 or less integer; 0 otherwise */
int isInteger(char *s){
	while(*s)
		if(*s >= '0' && *s <= '9'){
			s++;
		}
		else{
			return 0;
		}
	return 1;
}
