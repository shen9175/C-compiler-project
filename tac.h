#ifndef TAC_H
#define TAC_H

#include "symtab.h"
int newtempnumber;
int newlabelnumber;
FILE* yyout;
int StrOffset;
char** globalstring;
address getclosettableid(nodeptr t);
int istable(nodeptr t);
address tablelhs(nodeptr t);
address newtemp(struct typeinfo * t, SymbolTable st);
instrptr newinstr(int optype,address, address, address);
address newlabel();
instrptr concatecode( instrptr front,  instrptr end);
void codeGen_bool(nodeptr t,address trueDst,address falseDst);
void codeGen_break(nodeptr t ,address L_after);
void codeGen_continue(nodeptr t ,address L_top);
void codeGen_switch(nodeptr t, address t1, address L_after);
void codegen(nodeptr t);
void printaddress(address place);
void codePrint(nodeptr t);
int typesizecall(typeptr t);
address empty;


char* realaddress(address place);
void finalcode(nodeptr t);
#endif
