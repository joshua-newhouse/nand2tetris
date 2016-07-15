#ifndef VM_H
#define VM_H

#define MAXFILENAME 1000
#define p_arrInit(arr, n) 	\
	{int i;			\
	 for(i = 0; i < n; i++) \
		arr[i] = NULL;}

int nLine = 0;
const char *ext = ".vm";
int isfile;
DIR *dirptr = NULL;
struct dirent *a = NULL;
char **fileName;

int VM_NumFiles(const char *s);
void VM_OpenFiles(const char *s, FILE *fp[], int n, char *fileName[]);
void VM_CloseFiles(FILE *fp[], int n);

#endif
