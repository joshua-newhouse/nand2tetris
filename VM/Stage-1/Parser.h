#ifndef PARSER_H
#define PARSER_H

enum cmdType{
	C_ARITHMETIC, C_PUSH, C_POP, C_LABEL, C_GOTO, C_IF,
	C_FUNCTION, C_RETURN, C_CALL
};

typedef struct{
	enum cmdType commandType;
	char *arg1;
	char *arg2;
} Parser_command;

#define Parser_hasMoreCommands(s, len, fp) fgets(s, len, fp)
void Parser_Constructor(Parser_command *cmd);
int Parser_advance(char *s, Parser_command *cmd, int *fIndex);

#endif
