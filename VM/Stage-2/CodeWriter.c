#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Parser.h"
#include "CmdTables.h"
#include "CodeWriter.h"
#include "jlib.h"

#define STACK_MAX 1791
#define STACK_MIN 0
#define STATIC_VAR_RANGE 239	/* Range of available static mem locations (255 - 16) */
#define RAN_DIG 4		/* Number of characters required to write a string of STATIC_VAR_RANGE
					including \0 (239\0) */

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
/* PUSH:  writes asm code to push a value stored in D register to the stack */
#define PUSH(out) {strcat(out, "@SP\nA=M\nM=D\n");\
		   SP_INC(out);}
/* POP:  asm code to pop a value from the stack and put it in memSeg base address plus offset */
#define POP(out, memSeg) {SP_DEC(out);\
			  strcat(out, "@SP\nA=M\nD=M\n@R13\nM=D\n@" #memSeg "\nD=M\n@");\
			  strcat(out, cmd->arg2);\
			  strcat(out, "\nD=D+A\n@R14\nM=D\n@R13\nD=M\n@R14\nA=M\nM=D\n");}
/* SET_FALSE:  asm code to set false on the stack */
#define SET_FALSE "@SP\nA=M\nM=0\n"
/* SET_TRUE:  asm code to set true on the stack */
#define SET_TRUE "@SP\nA=M\nM=-1\n"
/* GetValAtAddr:  asm code to get the value at the address in the given memory segment plus offset 
			and store in D register */
#define GetValAtAddr(memSeg) {strcat(out, "@" #memSeg "\nD=M\n@");\
			      strcat(out, cmd->arg2);\
			      strcat(out, "\nA=D+A\nD=M\n");}
/* errCMD:  writes error message to console and terminates program */
#define errCMD(nLine, msg)	{printf("Error:  cannot write command on line %d\n%s\n", nLine, msg);\
			 	 exit(1);}

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

char *tempReg[8] = {"@R5\n", "@R6\n", "@R7\n", "@R8\n", "@R9\n", "@R10\n", "@R11\n", "@R12\n"};

extern int nLine;

int sp = 0;	/* Used to keep track of VM's stack pointer */

/* Keeps track of the number equality conditions */
int eqCond = 0;
int gtCond = 0;
int ltCond = 0;

void WritePushPop(Parser_command *cmd, char *out, int fileIndex, char *fileName[]);
void WriteArithmetic(Parser_command *cmd, char *out);
int Type(Parser_command *cmd, enum type t);
void WriteSymLab(char *out, const char *s, int x, enum symLab sl);
char *GenSym(int fileIndex, char *arg2, char *fileName[]);

#define initialize(output, memAddr, pointer) strcat(output, "@" #memAddr "\nD=A\n@" #pointer "\nM=D\n")
/* CW_InitializeVM:  initializes the virtual machine by setting up base mem addresses */
void CW_InitializeVM(char *output, FILE *fp){
	*output = '\0';
	initialize(output, 256, SP);		/* Set stack pointer */
	initialize(output, 2048, LCL);		/* Set lcl pointer */
	initialize(output, 3048, ARG);		/* Set arg pointer */	
	initialize(output, 4048, THIS);		/* Set this pointer */
	initialize(output, 5048, THAT);		/* Set that pointer */
	fputs(output, fp);
}

/* CW_TerminateVM:  terminates the virtual machine by setting and infinite loop */
void CW_TerminateVM(char *output, FILE *fp){
	*output = '\0';
	strcat(output, "(END)\n@END\n0;JMP\n");
	fputs(output, fp);
}

/* CW_WriteCode:  translates current parser command cmd into asm and writes it to file fp */
char *CW_WriteCode(Parser_command *cmd, char *output, FILE *fp, int fileIndex, char *fileName[]){
	*output = '\0';
	switch(cmd->commandType){
	case C_PUSH:
	case C_POP:
		WritePushPop(cmd, output, fileIndex, fileName);
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
void WritePushPop(Parser_command *cmd, char *out, int fileIndex, char *fileName[]){
	enum memorySegment a = Type(cmd, MEMORY);
	int isPush = cmd->commandType == C_PUSH;
	char *s;
	switch(a){
	case ARGUMENT:
		if(isPush){
			GetValAtAddr(ARG);
		}
		else{
			POP(out, ARG);
		}
		break;
	case LOCAL:
		if(isPush){
			GetValAtAddr(LCL);
		}
		else{
			POP(out, LCL);
		}
		break;
	case STATIC:
		s = GenSym(fileIndex, cmd->arg2, fileName);
		if(isPush){
			strcat(out, "@");
			strcat(out, s);
			strcat(out, "\nD=M\n");
		}
		else{
			SP_DEC(out);
			strcat(out, "@SP\nA=M\nD=M\n@");
			strcat(out, s);
			strcat(out, "\nM=D\n");
		}
		free(s);
		break;
	case CONSTANT:
		if(isPush){
			strcat(out, "@");
			strcat(out, cmd->arg2);
			strcat(out, "\n");
			strcat(out, "D=A\n");
		}
		else
			errCMD(nLine, "Cannot pop constant");
		break;
	case THIS:
		if(isPush){
			GetValAtAddr(THIS);
		}
		else{
			POP(out, THIS);
		}
		break;
	case THAT:
		if(isPush){
			GetValAtAddr(THAT);
		}
		else{
			POP(out, THAT);
		}
		break;
	case POINTER:
		switch(atoi(cmd->arg2)){
		case 0:
			s = "@R3\n";
			break;
		case 1:
			s = "@R4\n";
			break;
		default:
			errCMD(nLine, "Pointer out of bounds");
		}
		if(isPush){
			strcat(out, s);
			strcat(out, "D=M\n");
		}
		else{
			SP_DEC(out);
			strcat(out, "@SP\nA=M\nD=M\n");
			strcat(out, s);
			strcat(out, "M=D\n");				
		}
		break;
	case TEMP:
		s = tempReg[atoi(cmd->arg2)];
		if(isPush){
			strcat(out, s);
			strcat(out, "D=M\n");
		}
		else{
			SP_DEC(out);
			strcat(out, "@SP\nA=M\nD=M\n");
			strcat(out, s);
			strcat(out, "M=D\n");				
		}
		break;
	default:
		break;
	}
	if(isPush)
		PUSH(out);
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

/* MemSeg:  returns the memory segment or arith operation stored in command argument 1 */
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

char *GenSym(int fileIndex, char *arg2, char *fileName[]){
	char *t = (char *)malloc(strlen(fileName[fileIndex]) + 2 + strlen(arg2));
	if(t){
		*t = '\0';
		strcat(t, fileName[fileIndex]);
		strcat(t, ".");
		strcat(t, arg2);
		return t;
	}
	else{
		printf("Error:  memory could not be allocated for static variable on line %d\n", nLine);
		exit(40);
	}
}
