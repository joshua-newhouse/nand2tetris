#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assembler.h"
#define MAXLINE 300

int nLine = 0;
void OpenFiles(FILE **fp_s, FILE **fp_d, char *argv[]);
void CMDreset(CMD *cmd);

int main(int argc, char *argv[]){
	FILE *fp_source = NULL;
	FILE *fp_dest = NULL;
	char line[MAXLINE];
	char binStr[18] = {0};
	CMD command = {0};
	OpenFiles(&fp_source, &fp_dest, argv);
	while(fgets(line, MAXLINE, fp_source)){
		nLine++;
		Parse(line, &command, FIRST);
		CMDreset(&command);
	};
	rewind(fp_source);
	nLine = 0;
	CMDreset(&command);
	while(fgets(line, MAXLINE, fp_source)){
		nLine++;
		Parse(line, &command, SECOND);
		if(command.commandType == A_COMMAND || command.commandType == C_COMMAND){
			fputs(Code(&command, binStr), fp_dest);
		}
		CMDreset(&command);
	};
	
	clearSymTab();
	fclose(fp_source);
	fclose(fp_dest);
	return 0;
}

/* OpenFiles:  opens the source and destination files; the source file must exist in the current directory;
		the dest file will be created overwriting any file in the directory with the same name */
void OpenFiles(FILE **fp_s, FILE **fp_d, char *argv[]){
	char sName[100] = "./";
	if(argv[1]){
		strcat(sName, argv[1]);
	} else{
		printf("Error:  must enter source file name.\n");
		exit(11);
	}
	
	char dName[100] = "./";
	if(argv[2])
		strcat(dName, argv[2]);
	else{		
		strcat(dName, argv[1]);
		char *c = strstr(dName, "asm");
		strcpy(c, "hack");
	}	
	if(!(*fp_s = fopen(sName, "r"))){
		printf("Error:  source file could not be opened.  Check file name and ensure file exists in the current dir.\n");
		exit(12);
	}	
	if(!(*fp_d = fopen(dName, "w"))){
		printf("Error:  destination file could not be opened.\n");
		exit(13);
	}
}

/* CMDreset:  resets command structure to 0 */
void CMDreset(CMD *cmd){
	cmd->commandType = A_COMMAND;
	cmd->value = 0;
	cmd->symbol = NULL;
	cmd->dest = NULL;
	cmd->comp = NULL;
	cmd->jump = NULL;
}
