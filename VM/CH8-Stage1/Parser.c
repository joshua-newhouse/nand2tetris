#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FH.h"
#include "Parser.h"
#include "CmdTables.h"

#define DEBUG(expr) printf("Line " #expr "\n")

/* skipWhiteSpace:  skips leading white space in s */
#define skipWhiteSpace(s) {while((*s == ' ' || *s == '\t' || *s == '\0') && strPos--) s++;}

/* markStrEnd:  puts '\0' at the end of each word in the string s */
#define markStrEnd(s)  {int i = 0;\
			while(!isspace(*(s+i)) && *(s+i))\
				i++;\
			*(s+i) = '\0';}

/* moveToNextString:  moves sting pointer s to the next word in string s */
#define moveToNextStr(s) {while(*s++ && strPos--);	\
				strPos--;}

/* errCMD:  prints error message and terminates program */
#define errCMD(string, n, file, e) {printf("Error:  unknown command %s on line %d in <%s>\n", string, n, file);	\
			     exit(e);}

int isArith(char* s);
int isInteger(char* s);
int isGoodLabel(char* s);

/* Parser_Constructor:  initializes Parser_command structure to 0 */
void Parser_Constructor(Parser_command* cmd){
	cmd->commandType = 0;
	cmd->arg1 = NULL;
	cmd->arg2 = NULL;
}

/* Parser_advance: parses the current line and stores the info in the Parser_command structure */
int Parser_advance(char* s, Parser_command* cmd, FH_FileStruct inFile){
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
				errCMD(--s, inFile.nLine, inFile.fileName, 30);
		case '-':			/* Negative numbers not allowed */
			errCMD(s, inFile.nLine, inFile.fileName, 30);
		case '0': case '1': case '2': case '3': case '4':	/* Captures trailing number for certain commands */
		case '5': case '6': case '7': case '8': case '9':
			markStrEnd(s);
			if((cmd->commandType == C_PUSH || cmd->commandType == C_POP ||
			   cmd->commandType == C_FUNCTION || cmd->commandType == C_CALL) && isInteger(s)){
				cmd->arg2 = s;
				return 1;
			}
			else
				errCMD(s, inFile.nLine, inFile.fileName, 31);
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
			int l, g;
			if((l = strcmp(s, "label") == 0) || (g = strcmp(s,"goto") == 0) || strcmp(s,"if-goto") == 0){
				cmd->commandType = l ? C_LABEL : (g ? C_GOTO : C_IF);
				moveToNextStr(s);
				markStrEnd(s);
				cmd->arg1 = s;
				if(!isGoodLabel(s))
					errCMD(cmd->arg1, inFile.nLine, inFile.fileName, 300);
				return 1;
			}
			if(!(cmd->commandType == C_PUSH || cmd->commandType == C_POP || cmd->commandType == C_RETURN)){
				if((i = isArith(s)) && !cmd->arg1){	/* C_ARITHMETIC and assigns operation to arg 1 */
					cmd->commandType = C_ARITHMETIC;
					cmd->arg1 = arithTab[i-1];
					moveToNextStr(s);
				}
				else
					errCMD(s, inFile.nLine, inFile.fileName, 32);
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
				errCMD(s, inFile.nLine, inFile.fileName, 33);
			break;
		}
	}
}

/* isArith:  returns index plus 1 if s is an arithmetic command found in arithTab, 0 otherwise */
int isArith(char* s){
	int i;
	for(i = 0; arithTab[i]; i++){
		if(!strcmp(s, arithTab[i]))
			return i + 1;
	}
	return 0;
}

/* isInterger:  returns 1 if string s is a base 10 or less integer; 0 otherwise */
int isInteger(char* s){
	while(*s)
		if(*s >= '0' && *s <= '9'){
			s++;
		}
		else{
			return 0;
		}
	return 1;
}

/* isGoodLabel:  returns 1 if string s meets label guidlines; returns 0 otherwise */
int isGoodLabel(char* s){
	int i;
	for(i = 0; *s; i++, s++){
		if(!i && isdigit(*s))
			return 0;
		else if(isalnum(*s) || *s == '.' || *s == '_' || *s == '$' || *s == ':')
			printf("%c", *s);
		else
			return 0;
	}
	return 1;
}
