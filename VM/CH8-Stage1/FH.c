#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "FH.h"

#define MAXFILENAME 256

const char *ext = ".vm";
int isfile;
DIR *dirptr = NULL;
struct dirent *a = NULL;

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

/* FH_NumFiles:  determines number of files to be opened */
int FH_NumFiles(const char *s){
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

/* FH_OpenFiles:  opens all .vm files and the output .asm file in the specified directory */
void FH_OpenFiles(const char *s, FH_FileStruct files[], int n){
	files[0].fp = fopen("./a.asm", "w");
	files[0].fileName = "a.asm";
	files[0].nLine = 0;

	char fname[MAXFILENAME];
	if(isfile){
		if(*s == '.' && *(s+1) == '/')
			strcpy(fname, s);
		else{
			strcpy(fname, "./");
			strcat(fname, s);
		}
		files[1].nLine = 1;
		files[1].fp = fopen(fname, "r");
		files[1].fileName = (char *)malloc(strlen(fname) - 1);
		strcat(files[1].fileName, (fname + 2));	
	}
	else{
		int i = 1;
		while(a = readdir(dirptr))
			if(strend(a->d_name, ext)){
				files[i].fileName = (char *)malloc(strlen(a->d_name) + 1);
				strcpy(fname, s);
				strcat(fname, "/");
				strcat(files[i].fileName, a->d_name);
				strcat(fname, a->d_name);
				files[i].nLine = 1;
				files[i++].fp = fopen(fname, "r");
			}
	}

	int i;
	for(i = 0; i < n; i++)
		if(files[i].fp == NULL || files[i].fileName == NULL){
			printf("Error:  could not open file %d\n", i);
			exit(21);
		}
	closedir(dirptr);
}

/* FH_CloseFiles:  closes all files in fp[] */
void FH_CloseFiles(FH_FileStruct files[], int n){
	int i;
	for(i = 0; i < n; i++){
		if(i)
			free(files[i].fileName);
		fclose(files[i].fp);
	}
}
