#ifndef CW_H
#define CW_H

void CW_InitializeVM(char *output, FILE *fp);
void CW_TerminateVM(char *output, FILE *fp);
char *CW_WriteCode(Parser_command *cmd, char *output, FILE *fp);

#endif
