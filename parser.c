#include <stdio.h>
#include <stdlib.h>
#include "assembler.h"

extern int nLine;
char *GetSymbol(char *s, int type);
void CleanStrEnd(char *s);

/* Parse:  parses the current input line from the source file setting the
		fields of the cmd structure with the proper values */
void Parse(char *s, CMD *cmd){

	while(isspace(*s++))	/* skips leading white space */
		;
	--s;
	if(*s != '\n' && *s != '\0')
		CleanStrEnd(s);

	switch(*s){
	case '\0': case '\n':		/* skips blank lines */
		cmd->commandType = L_COMMAND;
		break;
	case '@':		/* sets A command and captures integer value or symbol */
		cmd->commandType = A_COMMAND;
		if(isdigit(*++s)){
			int i = 0;
			for(i = 0; *(s+i); i++)
				if(!isdigit(*(s+i))){
					printf("Error:  invalid symbol name %s on line %d\n", s, nLine);
					exit(1);
				}
			cmd->value = atoi(s);
		} else{
			cmd->symbol = GetSymbol(s, cmd->commandType);
		}
		break;
	case 'A': case 'D': case 'M': case '0':
	case '1': case '-': case '!':	/* sets C command and captures command destination, computation, and jump */
		cmd->commandType = C_COMMAND;
		char *sp = s;
		while(*s)
			switch(*s){
			case '=':
				*s = '\0';
				cmd->dest = sp;
				cmd->comp = ++s;
				break;
			case ';':
				*s = '\0';
				if(cmd->comp == NULL)
					cmd->comp = sp;
				if(*++s == 'J')
					cmd->jump = s;
				else if(*s){
					printf("Error:  unrecognized jump command \"%s\b\" on line %d\n", s, nLine);
					exit(2);
				}
				break;
			default:
				s++;
			}
		break;
	case '(':		/* sets L command and captures symbol */
		cmd->commandType = L_COMMAND;
		cmd->symbol = GetSymbol(++s, cmd->commandType);
		break;
	case '/':		/* skips comments */
		if(*(s+1) == '/'){
			cmd->commandType = L_COMMAND;
			break;
		}
	default:
		printf("Error:  unrecognized command \"%s\b\" on line %d\n", s, nLine);
		exit(3);
	}
}

/* GetSymbol:  captures symbol for both A and L commands */
char *GetSymbol(char *s, int type){
	int paren = 0;	
	if(type)
		paren = ')';

	int i;
	for(i = 0; *(s+i) != '\n' && *(s+i) != paren; i++){
		if(!isalpha(*(s+i)) && !isdigit(*(s+i)) && *(s+i) != '_' && *(s+i) != '.' && *(s+i) != '$' && *(s+i) != ':'){
			printf("Error:  invalid character %c in symbol name %s on line %d\n", *(s+i), s, nLine);
			exit(4);
		}
	}
	*(s+i) = '\0';
	return s;
}

/* CleanStrEnd:  removes comments and trailing white space from s */
void CleanStrEnd(char *s){
	while(*s != '\n' && *s != '\0'){
		if(*s == '/' && *(s+1) == '/')
			break;
		s++;
	}
	--s;
	while(*s == ' ' || *s == '\t')
		s--;
	*++s = '\0';
}
