#ifndef ASSEMBLER_H
#define ASSEMBLER_H

enum pass{
	FIRST, SECOND
};

enum cmdType{
	A_COMMAND, C_COMMAND, L_COMMAND
};

typedef struct command{
	enum cmdType commandType;
	unsigned int value;
	char *symbol;
	char *dest;
	char *comp;
	char *jump;
} CMD;

void Parse(char *s, CMD *cmd, enum pass);
char *Code(CMD *cmd, char binStr[]);
int addEntry(CMD *cmd);
void clearSymTab();
int getAddr(char *s);
void printSymTab();

#endif
