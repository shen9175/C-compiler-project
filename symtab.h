#ifndef SYMTAB_H
#define SYMTAB_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "cgram.tab.h"


extern int StrOffset;
extern FILE* yyout;
extern int yylex(void);
extern int yyparse();
extern char* yyname;
extern int line_num;
extern struct tree* yytreeroot;

typedef struct sym_table
{
	int nBuckets;
	int nEntries;
	struct sym_table *parent;
	int scope;
	struct sym_entry **tblist;
	int firstsixsize;//here is for param symbol table for x86_64 passing conventions
}* SymbolTable;

typedef struct
{
	int region;
	int offset;
	char* text;
	int seven;//here is only for the number of parameter that exceeds 6, it stores totalbytes_from_the_7th_to_before_current_one
	int ordernumber;
	int six[6];//this is information for initialize one function when moving first six parameters from special parameter registers to memory locations

//when insert symbol type of parameter list, the order is from first to last, when execute the code, the order is reversed, from last to first.
} address;


typedef struct code
{
	int op;
	address src1;
	address src2;
	address dest;
	struct code* head;
	struct code* tail;
}* instrptr;

typedef struct sym_entry
{
	SymbolTable table;
	char*s;
	int prototype;
	address place;
	address tableaddress;
	struct typeinfo *type;
	struct sym_entry *next;

}*SymbolTableEntry;


typedef struct typeinfo
{
	int basetype;
	union
	{
		struct array
			{
				int size;
				struct typeinfo* elemtype;
			}a;

		struct function
			{
				char*name;
				SymbolTable st;
				struct typeinfo* returntype;
				struct param *parameters;
			}f;
	}u;
} *typeptr;


typedef struct param
{
	char* name;
	struct typeinfo* type;
	struct param *next;

}*paramlist;







typedef struct tree
{
   int prodrule;
   int nkids;
   int value;
   char* svalue;
   SymbolTable st;
   struct typeinfo* type;
   struct tree **kids;
   struct token *leaf;
   instrptr code;
   address place;
}*nodeptr;

struct token
{
   int category;   /* the integer code returned by yylex */
   char *text;     /* the actual string (lexeme) matched */
   int lineno;     /* the line number on which the token occurs */
   char *filename; /* the source file in which the token occurs */
   int ival;       /* if you had an integer constant, store its value here */
   char *sval;      /* if you had a string constant, malloc space and store */
};











typedef struct
{
	SymbolTableEntry entry;
	int correct;
}boolSTE;



SymbolTable new_st(int size,int scope);
void delete_st(SymbolTable);
boolSTE insert_st(SymbolTable,char*,typeptr, int prototype);
SymbolTableEntry lookup_st(SymbolTable, char*);
SymbolTableEntry check_id_all_scope(char* s,SymbolTable st);
extern address empty;
extern int newlabelnumber;
extern int newtempnumber;
extern int semantic_error;
extern int StrOffset;
//extern char* globalstring;
extern SymbolTable globals;
extern SymbolTable current;
extern void printsymbols(SymbolTable st, int level);

extern typeptr integer_type;
extern typeptr string_type;
extern typeptr char_type;
extern typeptr array_type;
extern typeptr function_type;
extern typeptr error_type;
extern typeptr void_type;
extern typeptr print_type;
extern typeptr getchar_type;
extern typeptr table_type;
extern typeptr universal_type;
extern typeptr char_array_type;
void error(char* s,nodeptr t);
void warn(char* s,nodeptr t);
void populatesymbols(nodeptr t);
typeptr synthesize_type(nodeptr t);
void populate_init_declarators(nodeptr t, typeptr typ);
paramlist create_param_list(nodeptr t);
nodeptr peelparams(nodeptr t);
void populateparams(nodeptr t);
void populatelocals(nodeptr t);
char* get_funcname(nodeptr t);
void typechecking(nodeptr t);
paramlist create_argument_list(nodeptr t);
void checkfunctionparam(paramlist l,nodeptr t);
int compare_list_type(paramlist l1,paramlist l2,int n);
typeptr compare_types(typeptr t1,typeptr t2);
int get_subscript_value(nodeptr t);
void initial_types();
int iserrortype(typeptr t);
int isintegertype(typeptr t);
int ischartype(typeptr t);
int isstringtype(typeptr t);
int isarraytype(typeptr t);
int isfunctiontype(typeptr t);
int isvoidtype(typeptr t);
int istabletype(typeptr t);
void givescope(nodeptr t,SymbolTable st);
void addparamnumbers(nodeptr t,int nparam);

#define pushscope(stp) do{stp->parent=current;current=stp;}while(0)
#define popscope() do{current=current->parent;}while(0)

#define identifier 1000
#define constant 1002
#define primary_expression 1006
#define generic_selection 1011
#define generic_assoc_list 1012
#define generic_association 1014
#define postfix_expression 1016
#define argument_expression_list 1026
#define unary_expression 1028
#define unary_operator 1035
#define cast_expression 1041
#define multiplicative_expression 1043
#define additive_expression 1047
#define shift_expression 1050
#define relational_expression 1053
#define equality_expression 1058
#define and_expression 1061
#define exclusive_or_expression 1063
#define inclusive_or_expression 1065
#define logical_and_expression 1067
#define logical_or_expression 1069
#define conditional_expression 1071
#define assignment_expression 1073
#define assignment_operator 1075
#define expression 1086
#define constant_expression 1088
#define declaration 1089
#define declaration_specifiers 1092
#define init_declarator_list 1102
#define init_declarator 1104
#define storage_class_specifier 1106
#define type_specifier 1112
#define struct_or_union_specifier 1127
#define struct_or_union 1130
#define struct_declaration_list 1132
#define struct_declaration 1134
#define specifier_qualifier_list 1137
#define struct_declarator_list 1141
#define struct_declarator 1143
#define enum_specifier 1146
#define enumerator_list 1151
#define enumerator 1153
#define atomic_type_specifier 1155
#define type_qualifier 1156
#define function_specifier 1160
#define alignment_specifier 1162
#define declarator 1164
#define direct_declarator 1166
#define pointer 1180
#define type_qualifier_list 1184
#define parameter_type_list 1186
#define parameter_list 1188
#define parameter_declaration 1190
#define identifier_list 1193
#define type_name 1195
#define abstract_declarator 1197
#define direct_abstract_declarator 1200
#define initializer 1221
#define initializer_list 1224
#define designation 1228
#define designator_list 1229
#define designator 1231
#define static_assert_declaration 1233
#define statement 1234
#define labeled_statement 1240
#define compound_statement 1243
#define block_item_list 1245
#define block_item 1247
#define expression_statement 1249
#define selection_statement 1251
#define iteration_statement 1254
#define jump_statement 1268
#define translation_unit 1273
#define external_declaration 1275
#define function_definition 1277
#define declaration_list 1279
#define file 1281

#define GLOBAL 2000
#define LOCAL  2001
#define ARG    2023
#define PARAML 2002
#define IMM    2003
#define LABEL  2004
#define STR    2005
#define RET    2006
#define RETV   2007
#define ASSN   2008
#define BLT    2009
#define BGT    2010
#define BLE    2011
#define BGE    2012
#define BNE    2013
#define BEQ    2014
#define ADD    2015
#define SUB    2016
#define ADDR   2017
#define PARAMQ 2018
#define CALL   2019
#define LCONT  2020
#define NEG    2021
#define FUNC   2022
#define END    2023
#define PARAMB 2024
#define STOP   2025


#endif

