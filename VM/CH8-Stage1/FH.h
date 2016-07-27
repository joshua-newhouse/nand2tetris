#ifndef FH_H
#define FH_H

typedef struct{
	FILE* fp;
	char* fileName;
	int nLine;
} FH_FileStruct;

int FH_NumFiles(const char *s);
void FH_OpenFiles(const char *s, FH_FileStruct files[], int n);
void FH_CloseFiles(FH_FileStruct files[], int n);

#endif
