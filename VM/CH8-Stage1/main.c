#include <stdio.h>
#include "FH.h"
#include "Parser.h"
#include "CodeWriter.h"

#define MAXLINE 1000

#define DEBUG(expr) printf("Line " #expr "\n")

int main(int argc, char *argv[]){
	if(argc != 2){
		printf("Error:  usage \"VM.bin <source.vm>\" or \"VM.bin <dir>\" where <dir> contains .vm files.\n");
		return 1;
	}

	char input[MAXLINE];
	Parser_command cmd;
	char output[MAXLINE] = {0};

	/* Opening files */
	int nfiles = FH_NumFiles(argv[1]);
	FH_FileStruct files[nfiles];
	FH_OpenFiles(argv[1], files, nfiles);

	/* List files being translated */
	int j;
	for(j = 1; j < nfiles; j++)
		printf("%s\n", files[j].fileName);

	/* Initialize the virtual machine */
	CW_InitializeVM(output, files[0].fp);

	/* Begin translation */
	int fIndex = 1;
	while(Parser_hasMoreCommands(input, MAXLINE, files[fIndex].fp)){
		Parser_Constructor(&cmd);
		if(Parser_advance(input, &cmd, files[fIndex]))
			CW_WriteCode(&cmd, output, files[0].fp, files[fIndex]);
		files[fIndex].nLine++;
	}

	/* Terminate the virtual machine */
	CW_TerminateVM(output, files[0].fp);

	/* Closing files */
	FH_CloseFiles(files, nfiles);
	return 0;
}
