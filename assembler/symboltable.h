#define MAXARRAY 100	/* Maximum length of array linked list array */
#define ADDRBEG 16	/* Begining of symbol address domain */
#define MEMAVAIL 16368	/* Data memory locations available for symbols (16384(KBD) - 16(preDef) */

const char *predefSym[] = {
	"SP", "LCL", "ARG", "THIS", "THAT",
	"R0", "R1", "R2", "R3", "R4", "R5",
	"R6", "R7", "R8", "R9", "R10", "R11",
	"R12", "R13", "R14", "R15", "SCREEN",
	"KBD", NULL
};

const int predefAdr[] = {
	0, 1, 2, 3, 4,
	0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16384,
	24576, -1
};

typedef struct symbol{
	char *key;
	int addr;
	int nOccur;
	enum cmdType type;
	struct symbol *next;
} SYM;

SYM *symArray[MAXARRAY] = {NULL};

extern int nLine;
extern int nextIAddr;
int nextDAddr = ADDRBEG;	/* Next data address */
int freedAddr[MEMAVAIL] = {0};	/* Freed data address stack */
int fa_ptr = 0;			/* Pointer to top of freed address stack */

SYM *newEntry(CMD *cmd);
SYM *deleteEntry(SYM *p);
int strval(char *s);
