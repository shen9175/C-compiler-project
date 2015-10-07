#ifndef PARSETREE_H
#define PARSETREE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>






struct token
{
   int category;   /* the integer code returned by yylex */
   char *text;     /* the actual string (lexeme) matched */
   int lineno;     /* the line number on which the token occurs */
   char *filename; /* the source file in which the token occurs */
   int ival;       /* if you had an integer constant, store its value here */
   char *sval;      /* if you had a string constant, malloc space and store */
};




extern int yylex(void);
extern int yyparse();
extern char* yyname;
extern int line_num;
extern struct tree* yytreeroot;


#endif

