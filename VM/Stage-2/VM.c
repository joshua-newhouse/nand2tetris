#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "VM.h"
#include "Parser.h"
#include "CodeWriter.h"

#define MAXLINE 1000

int main(int argc, char *argv[]){
	if(argc != 2){
		printf("Error:  usage \"VM.bin <source.vm>\" or \"VM.bin <dir>\" where <dir> contains .vm files.\n");
		return 1;
	}

	char input[MAXLINE];
	Parser_command cmd;
	char output[MAXLINE] = {0};

	/* Opening files */
	int nfiles = VM_NumFiles(argv[1]);
	fileName = (char **)malloc(sizeof(char *) * nfiles);
	FILE *fp[nfiles];
	p_arrInit(fp, nfiles);
	VM_OpenFiles(argv[1], fp, nfiles, fileName);

	int j;
	for(j = 1; j < nfiles; j++)
		printf("%s\n", fileName[j]);

	/* Initialize the virtual machine */
	CW_InitializeVM(output, fp[0]);

	/* Begin translation */
	int fIndex = 1;
	while(Parser_hasMoreCommands(input, MAXLINE, fp[fIndex])){
		Parser_Constructor(&cmd);
		if(Parser_advance(input, &cmd, &fIndex))
			CW_WriteCode(&cmd, output, fp[0], fIndex, fileName);
		nLine++;
	}

	/* Terminate the virtual machine */
	CW_TerminateVM(output, fp[0]);

	int i;
	for(i = 1; i < nfiles; i++)
		free(*(fileName + i));
	free(fileName);

	/* Closing files */
	VM_CloseFiles(fp, nfiles);
	return 0;
}

/* strend:  checks that s ends with t; returns 1 if t is at end of s, 0 otherwise */
int strend(const char *s, const char *t){
	do{
		if(*s == *t)
			if(*t)
				t++;
			else
				return 1;
	} while(*s++);
	return 0;
}

/* VM_NumFiles:  determines number of files to be opened */
int VM_NumFiles(const char *s){
	int nf = 1;
	
	if(isfile = strend(s, ext))
		nf = 2;
	else{
		dirptr = opendir(s);
		if(dirptr){
			while(a = readdir(dirptr))
				if(strend(a->d_name, ext))
					nf++;
		}
		else{
			printf("Error:  cannot open %s.  Check that it is a .vm file or directory.\n", s);
			exit(20);
		}
		rewinddir(dirptr);
	}
	return nf;
}

/* VM_OpenFiles:  opens all .vm files and the output .asm file in the specified directory */
void VM_OpenFiles(const char *s, FILE *fp[], int n, char *fileName[]){
	fp[0] = fopen("./a.asm", "w");
	char fname[MAXFILENAME];
	if(isfile){
		if(*s == '.' && *(s+1) == '/')
			strcpy(fname, s);
		else{
			strcpy(fname, "./");
			strcat(fname, s);
		}
		fp[1] = fopen(fname, "r");
		fileName[1] = (char *)malloc(strlen(fname) - 1);
		strcat(fileName[1], (fname + 2));	
	}
	else{
		int i = 1;
		while(a = readdir(dirptr)){
			strcpy(fname, s);
			strcat(fname, "/");
			if(strend(a->d_name, ext)){
				fileName[i] = (char *)malloc(strlen(a->d_name) + 1);
				strcat(fileName[i], a->d_name);
				strcat(fname, a->d_name);
				fp[i++] = fopen(fname, "r");
			}
		}
	}

	int i;
	for(i = 0; i < n; i++)
		if(fp[i] == NULL){
			printf("Error:  could not open file %d.\n%p\n", i, fp[i]);
			exit(21);
		}
	closedir(dirptr);
}

/* VM_CloseFiles:  closes all files in fp[] */
void VM_CloseFiles(FILE *fp[], int n){
	int i;
	for(i = 0; i < n; i++)
		fclose(fp[i]);
}
