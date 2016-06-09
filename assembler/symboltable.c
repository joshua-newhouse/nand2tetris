#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assembler.h"
#include "symboltable.h"

/* addEntry:  adds symbol to symbol table; returns 1 if successful, 0 otherwise */
int addEntry(CMD *cmd){
	int i;
	for(i = 0; predefSym[i]; i++)
		if(!strcmp(predefSym[i], cmd->symbol))
			return 1;

	i = strval(cmd->symbol) % MAXARRAY;
	SYM *p;
	for(p = symArray[i]; p; p = p->next)
		if(!strcmp(p->key, cmd->symbol)){
			p->nOccur++;
			if(cmd->commandType == L_COMMAND && p->type == A_COMMAND){
				p->type = L_COMMAND;
				freedAddr[fa_ptr++] = p->addr;
				p->addr = nextIAddr;
			}
			return 1;
		}
	p = newEntry(cmd);
	if(p){
		p->next = symArray[i];
		symArray[i] = p;
		return 1;
	} else
		return 0;
}

/* newEntry:  returns pointer to new SYM object, initializes it with a key and an occurence */
SYM *newEntry(CMD *cmd){
	SYM *p = (SYM *)malloc(sizeof(*p));
	if(p){
		p->key = (char *)malloc(strlen(cmd->symbol) + 1);
		if(p->key){
			strcpy(p->key, cmd->symbol);
			p->type = cmd->commandType;
			if(p->type)
				p->addr = nextIAddr;
			else if(fa_ptr)
				p->addr = freedAddr[--fa_ptr];
			else
				p->addr = nextDAddr++;
			p->nOccur = 1;
			p->next = NULL;
			return p;
		} else{
			printf("Error: cannot alloc mem for %s on line %d\n", cmd->symbol, nLine);
			return NULL;
		}
	} else{
		printf("Error:  cannot alloc mem for %s on line %d\n", cmd->symbol, nLine);
		return NULL;
	}
}

/* deleteEntry:  deletes SYM object and returns pointer to the next SYM object */
SYM *deleteEntry(SYM *p){
	SYM *q = p->next;
	if(p->type == A_COMMAND)
		freedAddr[fa_ptr++] = p->addr;
	free(p->key);
	free(p);
	return q;
}

/* clearSymTab:  deletes all entries in the symbol table */
void clearSymTab(){
	SYM *p;
	int i;	
	for(i = 0; i < MAXARRAY; i++)		
		if(p = symArray[i])
			while(p = deleteEntry(p))
				;
}

/* getAddr:  returns address of given symbol key s; returns -1 if not found */
int getAddr(char *s){
	int i;
	for(i = 0; predefSym[i]; i++)
		if(!strcmp(predefSym[i], s))
			return predefAdr[i];
	int adr;
	i = strval(s) % MAXARRAY;
	SYM *p, *q;
	for(q = p = symArray[i]; p; q = p, p = p->next)
		if(!strcmp(p->key, s)){
			adr = p->addr;
			if(p->type == A_COMMAND && --p->nOccur == 0){
				if(p == symArray[i])
					symArray[i] = p->next;
				else
					q->next = p->next;
				deleteEntry(p);
			}
			return adr;
		}
	printf("Error:  no symbol %s on line %d\n", s, nLine);
	return -1;
}
		
/* strval:  returns sum of all characters in s */ 
int strval(char *s){
	int val = 0;	
	while(*s)
		val += *s++;
	return val;
}

void printSymTab(){
	int i;
	SYM *p;
	for(i = 0; i < MAXARRAY; i++)
		for(p = symArray[i]; p; p = p->next){
			printf("Key: %s\n", p->key);
			printf("Addr: %d\n", p->addr);
			printf("Occ: %d\n", p->nOccur);
			printf("Type: %d\n", p->type);
		}
}
			
		
