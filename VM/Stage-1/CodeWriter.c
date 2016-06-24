#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Parser.h"
#include "CmdTables.h"
#include "CodeWriter.h"
#include "jlib.h"

#define STACK_MAX 1791
#define STACK_MIN 0

/* SP_DEC:  writes asm code to decrement the stack pointer */
#define SP_DEC(out) {strcat(out, "@SP\nAM=M-1\n");\
		     if(--sp < STACK_MIN){\
			printf("Error:  stack pointer out of bounds on line %d\n", nLine);\
			exit(42);\
		     }}
/* SP_INC:  writes asm code to increment the stack pointer */
#define SP_INC(out) {strcat(out, "@SP\nM=M+1\n");\
		     if(++sp > STACK_MAX){\
			printf("Error:  stack pointer out of bounds on line %d\n", nLine);\
			exit(43);\
		     }}
/* SP_DEC:  writes asm code to push a value to the stack */
#define PUSH(out) {strcat(out, "D=A\n@SP\nA=M\nM=D\n");\
		   SP_INC(out);}
/* SET_FALSE:  asm code to set false on the stack */
#define SET_FALSE "@SP\nA=M\nM=0\n"
/* SET_TRUE:  asm code to set true on the stack */
#define SET_TRUE "@SP\nA=M\nM=-1\n"

#define DEBUG(expr) printf("Line " #expr "\n");

enum memorySegment{
	ARGUMENT, LOCAL, STATIC, CONSTANT,
	THIS, THAT, POINTER, TEMP
};
enum arithmetic{
	ADD, SUB, NEG, EQ, GT,
	LT, AND, OR, NOT
};
enum type{
	MEMORY, ARITHMETIC
};
enum symLab{
	SYMBOL, LABEL
};

extern int nLine;

int sp = 0;	/* Used to keep track of VM's stack pointer */

/* Keeps track of the number of equality conditions */
int eqCond = 0;
int gtCond = 0;
int ltCond = 0;

void WritePushPop(Parser_command *cmd, char *out);
void WriteArithmetic(Parser_command *cmd, char *out);
int Type(Parser_command *cmd, enum type t);
void WriteSymLab(char *out, const char *s, int x, enum symLab sl);

/* CW_InitializeVM:  initializes the virtual machine by setting up base mem addresses */
void CW_InitializeVM(char *output, FILE *fp){
	*output = '\0';
	strcat(output, "@256\nD=A\n@SP\nM=D\n");
	fputs(output, fp);
}

/* CW_TerminateVM:  terminates the virtual machine by setting and infinite loop */
void CW_TerminateVM(char *output, FILE *fp){
	*output = '\0';
	strcat(output, "(END)\n@END\n0;JMP\n");
	fputs(output, fp);
}

/* CW_WriteCode:  translates current parser command cmd into asm and writes it to file fp */
char *CW_WriteCode(Parser_command *cmd, char *output, FILE *fp){
	*output = '\0';
	switch(cmd->commandType){
	case C_PUSH:
	case C_POP:
		WritePushPop(cmd, output);
		break;
	case C_ARITHMETIC:
		WriteArithmetic(cmd, output);
		break;
	default:
		break;
	}
	fputs(output, fp);
}

/* WritePushPop:  translates push and pop commands */
void WritePushPop(Parser_command *cmd, char *out){
	if(cmd->commandType == C_PUSH){
		enum memorySegment a = Type(cmd, MEMORY);
		switch(a){
		case ARGUMENT:
			break;
		case LOCAL:
			break;
		case STATIC:
			break;
		case CONSTANT:
			strcat(out, "@");
			strcat(out, cmd->arg2);
			strcat(out, "\n");
			break;
		case THIS:
			break;
		case THAT:
			break;
		case POINTER:
			break;
		case TEMP:
			break;
		default:
			break;
		}
		PUSH(out);
	}

}

/* WriteArithmetic:  translates arithmetic commands */
void WriteArithmetic(Parser_command *cmd, char *out){
	enum arithmetic a = Type(cmd, ARITHMETIC);
	switch(a){
	case ADD:
		SP_DEC(out);
		strcat(out, "D=M\n");
		SP_DEC(out);
		strcat(out, "M=D+M\n");
		SP_INC(out)
		break;
	case SUB:
		SP_DEC(out);
		strcat(out, "D=M\n");
		SP_DEC(out);
		strcat(out, "M=M-D\n");
		SP_INC(out)
		break;
	case NEG:
		SP_DEC(out);
		strcat(out, "M=-M\n");
		SP_INC(out)
		break;
	case EQ:
		eqCond++;
		SP_DEC(out);
		strcat(out, "D=M\n");
		SP_DEC(out);
		strcat(out, "D=M-D\n");
		WriteSymLab(out, "EQ", eqCond, SYMBOL);
		strcat(out, "D;JEQ\n");
		strcat(out, SET_FALSE);
		WriteSymLab(out, "CONTINUE_EQ", eqCond, SYMBOL);
		strcat(out, "0;JMP\n");
		WriteSymLab(out, "EQ", eqCond, LABEL);
		strcat(out, SET_TRUE);
		WriteSymLab(out, "CONTINUE_EQ", eqCond, LABEL);
		SP_INC(out);
		break;
	case GT:
		gtCond++;
		SP_DEC(out);
		strcat(out, "D=M\n");
		SP_DEC(out);
		strcat(out, "D=M-D\n");
		WriteSymLab(out, "GT", gtCond, SYMBOL);
		strcat(out, "D;JGT\n");
		strcat(out, SET_FALSE);
		WriteSymLab(out, "CONTINUE_GT", gtCond, SYMBOL);
		strcat(out, "0;JMP\n");
		WriteSymLab(out, "GT", gtCond, LABEL);
		strcat(out, SET_TRUE);
		WriteSymLab(out, "CONTINUE_GT", gtCond, LABEL);
		SP_INC(out);
		break;
	case LT:
		ltCond++;
		SP_DEC(out);
		strcat(out, "D=M\n");
		SP_DEC(out);
		strcat(out, "D=M-D\n");
		WriteSymLab(out, "LT", ltCond, SYMBOL);
		strcat(out, "D;JLT\n");
		strcat(out, SET_FALSE);
		WriteSymLab(out, "CONTINUE_LT", ltCond, SYMBOL);
		strcat(out, "0;JMP\n");
		WriteSymLab(out, "LT", ltCond, LABEL);
		strcat(out, SET_TRUE);
		WriteSymLab(out, "CONTINUE_LT", ltCond, LABEL);
		SP_INC(out);
		break;
	case AND:
		SP_DEC(out);
		strcat(out, "D=M\n");
		SP_DEC(out);
		strcat(out, "M=D&M\n");
		SP_INC(out)
		break;
	case OR:
		SP_DEC(out);
		strcat(out, "D=M\n");
		SP_DEC(out);
		strcat(out, "M=D|M\n");
		SP_INC(out)
		break;
	case NOT:
		SP_DEC(out);
		strcat(out, "M=!M\n");
		SP_INC(out)
		break;
	default:
		break;
	}
}

/* MemSeg:  returns the memory segment or arithmetic op stored in command argument 1 */
int Type(Parser_command *cmd, enum type t){
	int i;
	for(i = 0; t ? arithTab[i] : memSegTab[i]; i++)
		if(!strcmp(t ? arithTab[i] : memSegTab[i], cmd->arg1))
			return i;
	return -1;
}

/* WriteSymLab:  creates and writes a new symbol, "@symbol", or label, "(label)",
		 to the output string out */
void WriteSymLab(char *out, const char *s, int x, enum symLab sl){
	char index[10];
	jlib_ItoB(x, index, 16);
	char *t = (char *)malloc(strlen(s) + strlen(index) + 2);
	if(t){
		*t = '\0';
		strcat(t, s);
		strcat(t, "_");
		strcat(t, index);
		switch(sl){
		case SYMBOL:
			strcat(out, "@");
			strcat(out, t);
			break;
		case LABEL:
			strcat(out, "(");
			strcat(out, t);
			strcat(out, ")");
			break;
		default:
			printf("Error:  not a symbol or label on line %d\n", nLine);
			break;
		}
		strcat(out, "\n");
		free(t);
	}
	else{
		printf("Error:  memory could not be allocated for symbol/label %s on line %d\n", s, nLine);
		exit(40);
	}
}
