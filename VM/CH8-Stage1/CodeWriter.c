#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FH.h"
#include "Parser.h"
#include "CmdTables.h"
#include "CodeWriter.h"
#include "jlib.h"

#define STACK_MAX 1791
#define STACK_MIN 0
#define STATIC_VAR_RANGE 239	/* Range of available static mem locations (255 - 16) */
#define RAN_DIG 4		/* Number of characters required to write a string of STATIC_VAR_RANGE
					including \0 (239\0) */

/* errCMD:  writes error message to console and terminates program */
#define errCMD(nLine, file, msg)	{printf("Error:  cannot write command on line %d in <%s>\n%s\n", nLine, file, msg);\
			 		 exit(1);}
/* SP_DEC:  writes asm code to decrement the stack pointer */
#define SP_DEC(out) {strcat(out, "@SP\nAM=M-1\n");\
		     if(--sp < STACK_MIN)\
			errCMD(inFile.nLine, inFile.fileName, "Stack pointer out of bounds\n");\
		     }
/* SP_INC:  writes asm code to increment the stack pointer */
#define SP_INC(out) {strcat(out, "@SP\nM=M+1\n");\
		     if(++sp > STACK_MAX)\
			errCMD(inFile.nLine, inFile.fileName, "Stack pointer out of bounds\n");\
		     }
/* PUSH:  writes asm code to push a value stored in D register to the stack */
#define PUSH(out) {strcat(out, "@SP\nA=M\nM=D\n");\
		   SP_INC(out);}
/* POP:  asm code to pop a value from the stack and put it in memSeg base address plus offset */
#define POP(out, memSeg, offset)	{SP_DEC(out);\
			  		strcat(out, "@SP\nA=M\nD=M\n@R13\nM=D\n@" #memSeg "\nD=M\n@");\
			  		strcat(out, offset);\
			  		strcat(out, "\nD=D+A\n@R14\nM=D\n@R13\nD=M\n@R14\nA=M\nM=D\n");}
/* SET_FALSE:  asm code to set false on the stack */
#define SET_FALSE "@SP\nA=M\nM=0\n"
/* SET_TRUE:  asm code to set true on the stack */
#define SET_TRUE "@SP\nA=M\nM=-1\n"
/* GetValAtAddr:  asm code to get the value at the address in the given memory segment plus offset 
			and store in D register */
#define GetValAtAddr(memSeg, offset) {strcat(out, "@" #memSeg "\nD=M\n@");\
			      strcat(out, offset);\
			      strcat(out, "\nA=D+A\nD=M\n");}

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

char* tempReg[8] = {"@R5\n", "@R6\n", "@R7\n", "@R8\n", "@R9\n", "@R10\n", "@R11\n", "@R12\n"};

int sp = 0;	/* Used to keep track of VM's stack pointer */

/* Keeps track of the number equality conditions */
int eqCond = 1;
int gtCond = 1;
int ltCond = 1;

void WritePushPop(Parser_command* cmd, char* out, FH_FileStruct inFile);
void WriteArithmetic(Parser_command* cmd, char* out, int nLine, FH_FileStruct inFile);
void WriteSymLab(Parser_command* cmd, char* out, const char* s, int x, FH_FileStruct inFile, enum symLab sl);
int Type(Parser_command* cmd, enum type t);
char* GenVar(char* arg2, char* fileName);

#define initialize(output, memAddr, pointer) strcat(output, "@" #memAddr "\nD=A\n@" #pointer "\nM=D\n")
/* CW_InitializeVM:  initializes the virtual machine by setting up base mem addresses */
void CW_InitializeVM(char* output, FILE* fp){
	*output = '\0';
	initialize(output, 256, SP);		/* Set stack pointer */
	initialize(output, 300, LCL);		/* Set lcl pointer */
	initialize(output, 400, ARG);		/* Set arg pointer */	
	initialize(output, 3000, THIS);		/* Set this pointer */
	initialize(output, 3010, THAT);		/* Set that pointer */
	fputs(output, fp);
}

/* CW_TerminateVM:  terminates the virtual machine by setting and infinite loop */
void CW_TerminateVM(char* output, FILE* fp){
	*output = '\0';
	strcat(output, "(END)\n@END\n0;JMP\n");
	fputs(output, fp);
}

/* CW_WriteCode:  translates current parser command cmd into asm and writes it to file fp */
char *CW_WriteCode(Parser_command *cmd, char *output, FILE *fp, FH_FileStruct inFile){
	*output = '\0';
	switch(cmd->commandType){
	case C_PUSH:
	case C_POP:
		WritePushPop(cmd, output, inFile);
		break;
	case C_ARITHMETIC:
		WriteArithmetic(cmd, output, inFile.nLine, inFile);
		break;
	case C_LABEL:
		WriteSymLab(cmd, output, "", 0, inFile, LABEL);
		break;
	case C_GOTO:
		WriteSymLab(cmd, output, "", 0, inFile, SYMBOL);
		strcat(output, "0;JMP\n");
		break;
	case C_IF:
		SP_DEC(output);
		strcat(output, "D=M\n");
		WriteSymLab(cmd, output, "", 0, inFile, SYMBOL);
		strcat(output, "D;JNE\n");
		break;
	default:
		break;
	}
	fputs(output, fp);
}

/* WritePushPop:  translates push and pop commands */
void WritePushPop(Parser_command *cmd, char *out, FH_FileStruct inFile){
	enum memorySegment a = Type(cmd, MEMORY);
	int isPush = cmd->commandType == C_PUSH;
	char *s;
	switch(a){
	case ARGUMENT:
		if(isPush){
			GetValAtAddr(ARG, cmd->arg2);
		}
		else{
			POP(out, ARG, cmd->arg2);
		}
		break;
	case LOCAL:
		if(isPush){
			GetValAtAddr(LCL, cmd->arg2);
		}
		else{
			POP(out, LCL, cmd->arg2);
		}
		break;
	case STATIC:
		if((s = GenVar(cmd->arg2, inFile.fileName)) == NULL){
			errCMD(inFile.nLine, inFile.fileName, "Cannot allocate memory for symbol");
		}
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
			errCMD(inFile.nLine, inFile.fileName, "Cannot pop constant");
		break;
	case THIS:
		if(isPush){
			GetValAtAddr(THIS, cmd->arg2);
		}
		else{
			POP(out, THIS, cmd->arg2);
		}
		break;
	case THAT:
		if(isPush){
			GetValAtAddr(THAT, cmd->arg2);
		}
		else{
			POP(out, THAT, cmd->arg2);
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
			errCMD(inFile.nLine, inFile.fileName, "Pointer out of bounds");
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
void WriteArithmetic(Parser_command *cmd, char *out, int nLine, FH_FileStruct inFile){
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
		WriteSymLab(cmd, out, "EQ", eqCond, inFile, SYMBOL);
		strcat(out, "D;JEQ\n");
		strcat(out, SET_FALSE);
		WriteSymLab(cmd, out, "CONTINUE_EQ", eqCond, inFile, SYMBOL);
		strcat(out, "0;JMP\n");
		WriteSymLab(cmd, out, "EQ", eqCond, inFile, LABEL);
		strcat(out, SET_TRUE);
		WriteSymLab(cmd, out, "CONTINUE_EQ", eqCond, inFile, LABEL);
		SP_INC(out);
		break;
	case GT:
		gtCond++;
		SP_DEC(out);
		strcat(out, "D=M\n");
		SP_DEC(out);
		strcat(out, "D=M-D\n");
		WriteSymLab(cmd, out, "GT", gtCond, inFile, SYMBOL);
		strcat(out, "D;JGT\n");
		strcat(out, SET_FALSE);
		WriteSymLab(cmd, out, "CONTINUE_GT", gtCond, inFile, SYMBOL);
		strcat(out, "0;JMP\n");
		WriteSymLab(cmd, out, "GT", gtCond, inFile, LABEL);
		strcat(out, SET_TRUE);
		WriteSymLab(cmd, out, "CONTINUE_GT", gtCond, inFile, LABEL);
		SP_INC(out);
		break;
	case LT:
		ltCond++;
		SP_DEC(out);
		strcat(out, "D=M\n");
		SP_DEC(out);
		strcat(out, "D=M-D\n");
		WriteSymLab(cmd, out, "LT", ltCond, inFile, SYMBOL);
		strcat(out, "D;JLT\n");
		strcat(out, SET_FALSE);
		WriteSymLab(cmd, out, "CONTINUE_LT", ltCond, inFile, SYMBOL);
		strcat(out, "0;JMP\n");
		WriteSymLab(cmd, out, "LT", ltCond, inFile, LABEL);
		strcat(out, SET_TRUE);
		WriteSymLab(cmd, out, "CONTINUE_LT", ltCond, inFile, LABEL);
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
void WriteSymLab(Parser_command* cmd, char* out, const char* s, int x, FH_FileStruct inFile, enum symLab sl){
	char index[10] = {0};
	char* t = NULL;
	if(x){
		jlib_ItoB(x, index, 16);
		t = (char*)malloc(strlen(s) + strlen(index) + 2);
	}
	else{
		t = (char*)malloc(strlen(cmd->arg1) + strlen(inFile.fileName) + 2);
	}
	if(t){
		switch(cmd->commandType){
		case C_ARITHMETIC:		
			*t = '\0';
			strcat(t, s);
			strcat(t, "_");
			strcat(t, index);
			break;
		case C_IF:
		case C_GOTO:
		case C_LABEL:
			*t = '\0';
			strcat(t, inFile.fileName);
			strcat(t, ":");
			strcat(t, cmd->arg1);
			break;
		}
	}
	else{
		printf("Error:  memory could not be allocated for symbol/label %s on line %d\n", s, inFile.nLine);
		exit(40);
	}
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
			printf("Error:  not a symbol or label on line %d\n", inFile.nLine);
			break;
	}
	strcat(out, "\n");
	free(t);
}

/* GenSym:  generates a static variable from a file name and symbol string */
char* GenVar(char* s, char* fileName){
	char* t = (char*)malloc(strlen(fileName) + 2 + strlen(s));
	*t = '\0';
	strcat(t, fileName);
	strcat(t, ".");
	strcat(t, s);
	return t;
}
