%{
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "symtab.h"

struct tree* AddTreeLeaf(int rule, struct token* tokenleaf);
struct tree* AddTreeNode(int rule,int kids,...);
void yyerror(char const *);

%}

%union {
struct token* tokptr;
struct tree* treeptr;
}


%token ICON FCON CCON ENUMERATION_CONSTANT TYPEDEF_NAME
%token IDENTIFIER STRING_LITERAL
%token SIZEOF
%token INCOP DECOP SHL SHR LE GE EQ NE
%token ANDAND OROR MUASN DIASN MOASN PLASN ASN
%token MIASN SLASN SRASN ANASN
%token ERASN ORASN
%token CM SM LT GT PLUS MINUS MUL DIV MOD LP RP LB RB LC RC COLON
%token QUEST AND OR ER NOT FOLLOW BANG DOT
%token POUND DPOUND

%token TYPEDEF EXTERN STATIC AUTO REGISTER 
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN RESTRICT INLINE

%token _ALIGNAS _ALIGNOF _ATOMIC _BOOL _COMPLEX _GENERIC _IMAGINARY _NORETURN _STATIC_ASSERT _THREAD_LOCAL TABLE

/* Lowest Precedence */
%left  LOWER_THAN_ELSE	//here define a lower precedence token which lower than ELSE token to tackle the classic "dangling else" problem to solve shift/reduce conflict
%left  ELSE

%left LOWER_THAN_LP
%left LP
/* Highest Precedence */


%type <tokptr> ICON FCON CCON ENUMERATION_CONSTANT TYPEDEF_NAME
%type <tokptr> IDENTIFIER STRING_LITERAL
%type <tokptr> SIZEOF
%type <tokptr> INCOP DECOP SHL SHR LE GE EQ NE
%type <tokptr> ANDAND OROR MUASN DIASN MOASN PLASN ASN
%type <tokptr> MIASN SLASN SRASN ANASN
%type <tokptr> ERASN ORASN
%type <tokptr> CM SM LT GT PLUS MINUS MUL DIV MOD LP RP LB RB LC RC COLON
%type <tokptr> QUEST AND OR ER NOT FOLLOW BANG DOT
%type <tokptr> POUND DPOUND

%type <tokptr> TYPEDEF EXTERN STATIC AUTO REGISTER 
%type <tokptr> CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%type <tokptr> STRUCT UNION ENUM ELIPSIS

%type <tokptr> CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN RESTRICT INLINE

%type <tokptr> _ALIGNAS _ALIGNOF _ATOMIC _BOOL _COMPLEX _GENERIC _IMAGINARY _NORETURN _STATIC_ASSERT _THREAD_LOCAL TABLE

%start file

%type <treeptr> identifier constant primary_expression generic_selection generic_assoc_list generic_association postfix_expression argument_expression_list
%type <treeptr> unary_expression unary_operator cast_expression multiplicative_expression additive_expression shift_expression relational_expression equality_expression
%type <treeptr> and_expression exclusive_or_expression inclusive_or_expression logical_and_expression logical_or_expression conditional_expression assignment_expression
%type <treeptr> assignment_operator expression constant_expression declaration declaration_specifiers init_declarator_list init_declarator storage_class_specifier
%type <treeptr> type_specifier struct_or_union_specifier struct_or_union struct_declaration_list struct_declaration specifier_qualifier_list struct_declarator_list
%type <treeptr> struct_declarator enum_specifier enumerator_list enumerator atomic_type_specifier type_qualifier function_specifier alignment_specifier declarator
%type <treeptr> direct_declarator pointer type_qualifier_list parameter_type_list parameter_list parameter_declaration identifier_list type_name abstract_declarator
%type <treeptr> direct_abstract_declarator /*typedef_name*/ initializer initializer_list designation designator_list designator static_assert_declaration statement
%type <treeptr> labeled_statement compound_statement block_item_list block_item expression_statement selection_statement iteration_statement jump_statement
%type <treeptr> translation_unit external_declaration function_definition declaration_list file

%%


identifier
	:IDENTIFIER			{$$=AddTreeLeaf(1000,$1);}
	|ENUMERATION_CONSTANT		{$$=AddTreeLeaf(1001,$1);}
	;


constant
	:ICON			{$$=AddTreeLeaf(1002,$1);}
	|FCON			{$$=AddTreeLeaf(1003,$1);}
	|ENUMERATION_CONSTANT	{$$=AddTreeLeaf(1004,$1);}
	|CCON			{$$=AddTreeLeaf(1005,$1);}
	;

primary_expression
	:IDENTIFIER		{$$=AddTreeLeaf(1006,$1);}
	|constant		{$$=AddTreeNode(1007,1,$1);}
	|STRING_LITERAL 	{$$=AddTreeLeaf(1008,$1);}
	|LP expression RP	{$$=AddTreeNode(1009,3,AddTreeLeaf(1009,$1),$2,AddTreeLeaf(1009,$3));}
	|generic_selection	{$$=AddTreeNode(1010,1,$1);}
	;



generic_selection
	:_GENERIC LP assignment_expression CM generic_assoc_list RP	{$$=AddTreeNode(1011,6,AddTreeLeaf(1011,$1),AddTreeLeaf(1011,$2),$3,AddTreeLeaf(1011,$4),$5,AddTreeLeaf(1011,$6));}
	;

generic_assoc_list
	:generic_association				{$$=AddTreeNode(1012,1,$1);}
	|generic_assoc_list CM generic_association 	{$$=AddTreeNode(1013,3,$1,AddTreeLeaf(1013,$2),$3);}
	;

generic_association
	:type_name COLON assignment_expression		{$$=AddTreeNode(1014,3,$1,AddTreeLeaf(1014,$2),$3);}
	|DEFAULT   COLON assignment_expression		{$$=AddTreeNode(1015,3,AddTreeLeaf(1015,$1),AddTreeLeaf(1015,$2),$3);}
	;

postfix_expression
	:primary_expression					{$$=AddTreeNode(1016,1,$1);}
	|postfix_expression LB expression RB	 		{$$=AddTreeNode(1017,4,$1,AddTreeLeaf(1017,$2),$3,AddTreeLeaf(1017,$4));}
	|postfix_expression LP argument_expression_list RP 	{$$=AddTreeNode(1018,4,$1,AddTreeLeaf(1018,$2),$3,AddTreeLeaf(1018,$4));}
	|postfix_expression LP RP				{$$=AddTreeNode(1019,3,$1,AddTreeLeaf(1019,$2),AddTreeLeaf(1019,$3));}
	|postfix_expression DOT identifier			{$$=AddTreeNode(1020,3,$1,AddTreeLeaf(1020,$2),$3);}
	|postfix_expression FOLLOW identifier			{$$=AddTreeNode(1021,3,$1,AddTreeLeaf(1021,$2),$3);}
	|postfix_expression INCOP				{$$=AddTreeNode(1022,2,$1,AddTreeLeaf(1022,$2));}
	|postfix_expression DECOP				{$$=AddTreeNode(1023,2,$1,AddTreeLeaf(1023,$2));}
	|LP type_name RP LC initializer_list RC			{$$=AddTreeNode(1024,6,AddTreeLeaf(1024,$1),$2,AddTreeLeaf(1024,$3),AddTreeLeaf(1024,$4),$5,AddTreeLeaf(1024,$6));}
	|LP type_name RP LC initializer_list CM RC		{$$=AddTreeNode(1025,7,AddTreeLeaf(1025,$1),$2,AddTreeLeaf(1025,$3),AddTreeLeaf(1025,$4),$5,AddTreeLeaf(1025,$6),AddTreeLeaf(1025,$7));}
	;

argument_expression_list
	:assignment_expression					{$$=AddTreeNode(1026,1,$1);}
	|argument_expression_list CM assignment_expression	{$$=AddTreeNode(1027,3,$1,AddTreeLeaf(1027,$2),$3);}
	;

unary_expression
	:postfix_expression		{$$=AddTreeNode(1028,1,$1);}
	|INCOP unary_expression 	{$$=AddTreeNode(1029,2,AddTreeLeaf(1029,$1),$2);}
	|DECOP unary_expression 	{$$=AddTreeNode(1030,2,AddTreeLeaf(1030,$1),$2);}
	|unary_operator cast_expression	{$$=AddTreeNode(1031,2,$1,$2);}
	|SIZEOF unary_expression	{$$=AddTreeNode(1032,2,AddTreeLeaf(1032,$1),$2);}
	|SIZEOF LP type_name RP		{$$=AddTreeNode(1033,4,AddTreeLeaf(1033,$1),AddTreeLeaf(1033,$2),$3,AddTreeLeaf(1033,$4));}
	|_ALIGNOF LP type_name RP	{$$=AddTreeNode(1034,4,AddTreeLeaf(1034,$1),AddTreeLeaf(1034,$2),$3,AddTreeLeaf(1034,$4));}
	;

unary_operator
	:AND	{$$=AddTreeLeaf(1035,$1);}
	|MUL	{$$=AddTreeLeaf(1036,$1);}
	|PLUS	{$$=AddTreeLeaf(1037,$1);}
	|MINUS	{$$=AddTreeLeaf(1038,$1);}
	|NOT	{$$=AddTreeLeaf(1039,$1);}
	|BANG	{$$=AddTreeLeaf(1040,$1);}
	;

cast_expression
	:unary_expression			{$$=AddTreeNode(1041,1,$1);}
	|LP type_name RP cast_expression	{$$=AddTreeNode(1042,4,AddTreeLeaf(1042,$1),$2,AddTreeLeaf(1042,$3),$4);}
	;

multiplicative_expression
	:cast_expression					{$$=AddTreeNode(1043,1,$1);}
	|multiplicative_expression MUL cast_expression		{$$=AddTreeNode(1044,3,$1,AddTreeLeaf(1044,$2),$3);}
	|multiplicative_expression DIV cast_expression		{$$=AddTreeNode(1045,3,$1,AddTreeLeaf(1045,$2),$3);}
	|multiplicative_expression MOD cast_expression		{$$=AddTreeNode(1046,3,$1,AddTreeLeaf(1046,$2),$3);}
	;

additive_expression
	:multiplicative_expression				{$$=AddTreeNode(1047,1,$1);}
	|additive_expression PLUS multiplicative_expression	{$$=AddTreeNode(1048,3,$1,AddTreeLeaf(1048,$2),$3);}
	|additive_expression MINUS multiplicative_expression	{$$=AddTreeNode(1049,3,$1,AddTreeLeaf(1049,$2),$3);}
	;

shift_expression
	:additive_expression					{$$=AddTreeNode(1050,1,$1);}
	|shift_expression SHL additive_expression		{$$=AddTreeNode(1051,3,$1,AddTreeLeaf(1051,$2),$3);}
	|shift_expression SHR additive_expression		{$$=AddTreeNode(1052,3,$1,AddTreeLeaf(1052,$2),$3);}
	;

relational_expression
	:shift_expression					{$$=AddTreeNode(1053,1,$1);}
	|relational_expression LT shift_expression		{$$=AddTreeNode(1054,3,$1,AddTreeLeaf(1054,$2),$3);}
	|relational_expression GT shift_expression		{$$=AddTreeNode(1055,3,$1,AddTreeLeaf(1055,$2),$3);}
	|relational_expression LE shift_expression		{$$=AddTreeNode(1056,3,$1,AddTreeLeaf(1056,$2),$3);}
	|relational_expression GE shift_expression		{$$=AddTreeNode(1057,3,$1,AddTreeLeaf(1057,$2),$3);}
	;

equality_expression
	:relational_expression					{$$=AddTreeNode(1058,1,$1);}
	|equality_expression EQ relational_expression		{$$=AddTreeNode(1059,3,$1,AddTreeLeaf(1059,$2),$3);}	
	|equality_expression NE relational_expression		{$$=AddTreeNode(1060,3,$1,AddTreeLeaf(1060,$2),$3);}
	;

and_expression
	:equality_expression					{$$=AddTreeNode(1061,1,$1);}
	|and_expression AND equality_expression			{$$=AddTreeNode(1062,3,$1,AddTreeLeaf(1062,$2),$3);}
	;

exclusive_or_expression
	:and_expression						{$$=AddTreeNode(1063,1,$1);}
	|exclusive_or_expression ER and_expression		{$$=AddTreeNode(1064,3,$1,AddTreeLeaf(1064,$2),$3);}
	;

inclusive_or_expression
	:exclusive_or_expression				{$$=AddTreeNode(1065,1,$1);}
	|inclusive_or_expression OR exclusive_or_expression	{$$=AddTreeNode(1066,3,$1,AddTreeLeaf(1066,$2),$3);}
	;

logical_and_expression
	:inclusive_or_expression				{$$=AddTreeNode(1067,1,$1);}
	|logical_and_expression ANDAND inclusive_or_expression	{$$=AddTreeNode(1068,3,$1,AddTreeLeaf(1068,$2),$3);}
	;

logical_or_expression
	:logical_and_expression					{$$=AddTreeNode(1069,1,$1);}
	|logical_or_expression OROR logical_and_expression	{$$=AddTreeNode(1070,3,$1,AddTreeLeaf(1070,$2),$3);}
	;

conditional_expression
	:logical_or_expression							{$$=AddTreeNode(1071,1,$1);}
	|logical_or_expression QUEST expression COLON conditional_expression	{$$=AddTreeNode(1072,5,$1,AddTreeLeaf(1072,$2),$3,AddTreeLeaf(1072,$4),$5);}
	;

assignment_expression
	:conditional_expression							{$$=AddTreeNode(1073,1,$1);}
	|unary_expression assignment_operator assignment_expression		{$$=AddTreeNode(1074,3,$1,$2,$3);}
	;

assignment_operator
	:ASN			{$$=AddTreeLeaf(1075,$1);}
	|MUASN			{$$=AddTreeLeaf(1076,$1);}
	|DIASN			{$$=AddTreeLeaf(1077,$1);}
	|MOASN			{$$=AddTreeLeaf(1078,$1);}
	|PLASN			{$$=AddTreeLeaf(1079,$1);}
	|MIASN			{$$=AddTreeLeaf(1080,$1);}
	|SLASN			{$$=AddTreeLeaf(1081,$1);}
	|SRASN			{$$=AddTreeLeaf(1082,$1);}
	|ANASN			{$$=AddTreeLeaf(1083,$1);}
	|ERASN			{$$=AddTreeLeaf(1084,$1);}
	|ORASN			{$$=AddTreeLeaf(1085,$1);}
	;	

expression
	:assignment_expression				{$$=AddTreeNode(1086,1,$1);}
	|expression CM assignment_expression		{$$=AddTreeNode(1087,3,$1,AddTreeLeaf(1087,$2),$3);}
	;

constant_expression
	:conditional_expression				{$$=AddTreeNode(1088,1,$1);}
	;

declaration
	:declaration_specifiers init_declarator_list SM	{$$=AddTreeNode(1089,3,$1,$2,AddTreeLeaf(1089,$3));}
	|declaration_specifiers SM			{$$=AddTreeNode(1090,2,$1,AddTreeLeaf(1090,$2));}
	|static_assert_declaration			{$$=AddTreeNode(1091,1,$1);}
	;

declaration_specifiers
	:storage_class_specifier declaration_specifiers	{$$=AddTreeNode(1092,2,$1,$2);}
	|storage_class_specifier			{$$=AddTreeNode(1093,1,$1);}
	|type_specifier	declaration_specifiers		{$$=AddTreeNode(1094,2,$1,$2);}
	|type_specifier					{$$=AddTreeNode(1095,1,$1);}
	|type_qualifier declaration_specifiers		{$$=AddTreeNode(1096,2,$1,$2);}
	|type_qualifier					{$$=AddTreeNode(1097,1,$1);}
	|function_specifier declaration_specifiers	{$$=AddTreeNode(1098,2,$1,$2);}
	|function_specifier				{$$=AddTreeNode(1099,1,$1);}
	|alignment_specifier declaration_specifiers	{$$=AddTreeNode(1100,2,$1,$2);}
	|alignment_specifier				{$$=AddTreeNode(1101,1,$1);}
	;

init_declarator_list
	:init_declarator				{$$=AddTreeNode(1102,1,$1);}
	|init_declarator_list CM init_declarator	{$$=AddTreeNode(1103,3,$1,AddTreeLeaf(1103,$2),$3);}
	;

init_declarator
	:declarator					{$$=AddTreeNode(1104,1,$1);}
	|declarator ASN initializer			{$$=AddTreeNode(1105,3,$1,AddTreeLeaf(1105,$2),$3);}
	;

storage_class_specifier
	:TYPEDEF					{$$=AddTreeLeaf(1106,$1);}
	|EXTERN						{$$=AddTreeLeaf(1107,$1);}
	|STATIC						{$$=AddTreeLeaf(1108,$1);}
	|_THREAD_LOCAL					{$$=AddTreeLeaf(1109,$1);}
	|AUTO						{$$=AddTreeLeaf(1110,$1);}
	|REGISTER					{$$=AddTreeLeaf(1111,$1);}
	;



type_specifier
	:VOID						{$$=AddTreeLeaf(1112,$1);}
	|CHAR						{$$=AddTreeLeaf(1113,$1);}
	|SHORT						{$$=AddTreeLeaf(1114,$1);}
	|INT						{$$=AddTreeLeaf(1115,$1);}
	|LONG						{$$=AddTreeLeaf(1116,$1);}
	|FLOAT						{$$=AddTreeLeaf(1117,$1);}
	|DOUBLE						{$$=AddTreeLeaf(1118,$1);}
	|SIGNED						{$$=AddTreeLeaf(1119,$1);}
	|UNSIGNED					{$$=AddTreeLeaf(1120,$1);}
	|_BOOL						{$$=AddTreeLeaf(1121,$1);}
	|_COMPLEX					{$$=AddTreeLeaf(1122,$1);}
	|atomic_type_specifier				{$$=AddTreeNode(1123,1,$1);}
	|struct_or_union_specifier			{$$=AddTreeNode(1124,1,$1);}
	|enum_specifier					{$$=AddTreeNode(1125,1,$1);}
	|TYPEDEF_NAME					{$$=AddTreeLeaf(1126,$1);}
	|TABLE						{$$=AddTreeLeaf(1282,$1);}
	;



struct_or_union_specifier
	:struct_or_union identifier LC struct_declaration_list RC	{$$=AddTreeNode(1127,5,$1,$2,AddTreeLeaf(1127,$3),$4,AddTreeLeaf(1127,$5));}
	|struct_or_union LC struct_declaration_list RC			{$$=AddTreeNode(1128,4,$1,AddTreeLeaf(1128,$2),$3,AddTreeLeaf(1128,$4));}
	|struct_or_union identifier					{$$=AddTreeNode(1129,2,$1,$2);}
	;

struct_or_union
	:STRUCT								{$$=AddTreeLeaf(1130,$1);}
	|UNION								{$$=AddTreeLeaf(1131,$1);}
	;

struct_declaration_list
	:struct_declaration						{$$=AddTreeNode(1132,1,$1);}
	|struct_declaration_list struct_declaration			{$$=AddTreeNode(1133,2,$1,$2);}
	;

struct_declaration
	:specifier_qualifier_list struct_declarator_list SM		{$$=AddTreeNode(1134,3,$1,$2,AddTreeLeaf(1134,$3));}
	|specifier_qualifier_list SM					{$$=AddTreeNode(1135,2,$1,AddTreeLeaf(1135,$2));}
	|static_assert_declaration					{$$=AddTreeNode(1136,1,$1);}
	;

specifier_qualifier_list
	:type_specifier specifier_qualifier_list			{$$=AddTreeNode(1137,2,$1,$2);}
	|type_specifier							{$$=AddTreeNode(1138,1,$1);}
	|type_qualifier specifier_qualifier_list			{$$=AddTreeNode(1139,2,$1,$2);}
	|type_qualifier							{$$=AddTreeNode(1140,1,$1);}
	;

struct_declarator_list
	:struct_declarator						{$$=AddTreeNode(1141,1,$1);}
	|struct_declarator_list CM struct_declarator			{$$=AddTreeNode(1142,3,$1,AddTreeLeaf(1142,$2),$3);}
	;

struct_declarator
	:declarator							{$$=AddTreeNode(1143,1,$1);}
	|declarator COLON constant_expression				{$$=AddTreeNode(1144,3,$1,AddTreeLeaf(1144,$2),$3);}
	|COLON constant_expression					{$$=AddTreeNode(1145,2,AddTreeLeaf(1145,$1),$2);}
	;

enum_specifier
	:ENUM identifier LC enumerator_list RC				{$$=AddTreeNode(1146,5,AddTreeLeaf(1146,$1),$2,AddTreeLeaf(1146,$3),$4,AddTreeLeaf(1146,$5));}
	|ENUM LC enumerator_list RC					{$$=AddTreeNode(1147,4,AddTreeLeaf(1147,$1),AddTreeLeaf(1147,$2),$3,AddTreeLeaf(1147,$4));}
	|ENUM identifier LC enumerator_list CM RC			{$$=AddTreeNode(1148,6,AddTreeLeaf(1148,$1),$2,AddTreeLeaf(1148,$3),$4,AddTreeLeaf(1148,$5),AddTreeLeaf(1148,$6));}
	|ENUM LC enumerator_list CM RC					{$$=AddTreeNode(1149,5,AddTreeLeaf(1149,$1),AddTreeLeaf(1149,$2),$3,AddTreeLeaf(1149,$4),AddTreeLeaf(1149,$5));}
	|ENUM identifier						{$$=AddTreeNode(1150,2,AddTreeLeaf(1150,$1),$2);}
	;

enumerator_list
	:enumerator							{$$=AddTreeNode(1151,1,$1);}
	|enumerator_list CM enumerator					{$$=AddTreeNode(1152,3,$1,AddTreeLeaf(1152,$2),$3);}
	;

enumerator
	:IDENTIFIER							{$$=AddTreeLeaf(1153,$1);}
	|IDENTIFIER ASN constant_expression				{$$=AddTreeNode(1154,3,AddTreeLeaf(1154,$1),AddTreeLeaf(1154,$2),$3);}
	;



atomic_type_specifier
	:_ATOMIC LP type_name RP 					{$$=AddTreeNode(1155,4,AddTreeLeaf(1155,$1),AddTreeLeaf(1155,$2),$3,AddTreeLeaf(1155,$4));}
	;

type_qualifier
	:CONST								{$$=AddTreeLeaf(1156,$1);}
	|RESTRICT							{$$=AddTreeLeaf(1157,$1);}
	|VOLATILE							{$$=AddTreeLeaf(1158,$1);}
	|_ATOMIC %prec LOWER_THAN_LP					{$$=AddTreeLeaf(1159,$1);}
	;

function_specifier
	:INLINE								{$$=AddTreeLeaf(1160,$1);}
	|_NORETURN							{$$=AddTreeLeaf(1161,$1);}
	;

alignment_specifier
	:_ALIGNAS LP type_name RP					{$$=AddTreeNode(1162,4,AddTreeLeaf(1162,$1),AddTreeLeaf(1162,$2),$3,AddTreeLeaf(1162,$4));}
	|_ALIGNAS LP constant_expression RP				{$$=AddTreeNode(1163,4,AddTreeLeaf(1163,$1),AddTreeLeaf(1163,$2),$3,AddTreeLeaf(1163,$4));}
	;

declarator
	:pointer direct_declarator					{$$=AddTreeNode(1164,2,$1,$2);}
	|direct_declarator						{$$=AddTreeNode(1165,1,$1);}
	;

direct_declarator
	:identifier									{$$=AddTreeNode(1166,1,$1);}
	|LP declarator RP								{$$=AddTreeNode(1167,3,AddTreeLeaf(1167,$1),$2,AddTreeLeaf(1167,$3));}
	|direct_declarator LB type_qualifier_list assignment_expression RB		{$$=AddTreeNode(1168,5,$1,AddTreeLeaf(1168,$2),$3,$4,AddTreeLeaf(1168,$5));}
	|direct_declarator LB assignment_expression RB					{$$=AddTreeNode(1169,4,$1,AddTreeLeaf(1169,$2),$3,AddTreeLeaf(1169,$4));}
	|direct_declarator LB type_qualifier_list RB					{$$=AddTreeNode(1170,4,$1,AddTreeLeaf(1170,$2),$3,AddTreeLeaf(1170,$4));}
	|direct_declarator LB RB							{$$=AddTreeNode(1171,3,$1,AddTreeLeaf(1171,$2),AddTreeLeaf(1171,$3));}
	|direct_declarator LB STATIC type_qualifier_list assignment_expression RB	{$$=AddTreeNode(1172,6,$1,AddTreeLeaf(1172,$2),AddTreeLeaf(1172,$3),$4,$5,AddTreeLeaf(1172,$6));}
	|direct_declarator LB STATIC assignment_expression RB				{$$=AddTreeNode(1173,5,$1,AddTreeLeaf(1173,$2),AddTreeLeaf(1173,$3),$4,AddTreeLeaf(1173,$5));}
	|direct_declarator LB type_qualifier_list STATIC assignment_expression RB	{$$=AddTreeNode(1174,6,$1,AddTreeLeaf(1174,$2),$3,AddTreeLeaf(1174,$4),$5,AddTreeLeaf(1174,$6));}
	|direct_declarator LB type_qualifier_list MUL RB				{$$=AddTreeNode(1175,5,$1,AddTreeLeaf(1175,$2),$3,AddTreeLeaf(1175,$4),AddTreeLeaf(1175,$5));}
	|direct_declarator LB MUL RB							{$$=AddTreeNode(1176,4,$1,AddTreeLeaf(1176,$2),AddTreeLeaf(1176,$3),AddTreeLeaf(1176,$4));}
	|direct_declarator LP parameter_type_list RP					{$$=AddTreeNode(1177,4,$1,AddTreeLeaf(1177,$2),$3,AddTreeLeaf(1177,$4));}
	|direct_declarator LP identifier_list RP					{$$=AddTreeNode(1178,4,$1,AddTreeLeaf(1178,$2),$3,AddTreeLeaf(1178,$4));}
	|direct_declarator LP RP							{$$=AddTreeNode(1179,3,$1,AddTreeLeaf(1179,$2),AddTreeLeaf(1179,$3));}
	;

pointer
	:MUL type_qualifier		{$$=AddTreeNode(1180,1,$1);}
	|MUL				{$$=AddTreeLeaf(1181,$1);}
	|MUL type_qualifier pointer	{$$=AddTreeNode(1182,3,AddTreeLeaf(1182,$1),$2,$3);}
	|MUL pointer 			{$$=AddTreeNode(1183,2,AddTreeLeaf(1183,$1),$2);}
	;

type_qualifier_list
	:type_qualifier					{$$=AddTreeNode(1184,1,$1);}
	|type_qualifier_list type_qualifier		{$$=AddTreeNode(1185,2,$1,$2);}
	;

parameter_type_list
	:parameter_list					{$$=AddTreeNode(1186,1,$1);}
	|parameter_list CM ELIPSIS			{$$=AddTreeNode(1187,3,$1,AddTreeLeaf(1187,$2),AddTreeLeaf(1187,$3));}
	;

parameter_list
	:parameter_declaration				{$$=AddTreeNode(1188,1,$1);}
	|parameter_list CM parameter_declaration	{$$=AddTreeNode(1189,3,$1,AddTreeLeaf(1189,$2),$3);}
	;

parameter_declaration
	:declaration_specifiers declarator		{$$=AddTreeNode(1190,2,$1,$2);}
	|declaration_specifiers abstract_declarator	{$$=AddTreeNode(1191,2,$1,$2);}
	|declaration_specifiers				{$$=AddTreeNode(1192,1,$1);}
	;

identifier_list
	:IDENTIFIER					{$$=AddTreeLeaf(1193,$1);}
	|identifier_list CM IDENTIFIER			{$$=AddTreeNode(1194,3,$1,AddTreeLeaf(1194,$2),AddTreeLeaf(1194,$3));}
	;

type_name
	:specifier_qualifier_list abstract_declarator	{$$=AddTreeNode(1195,2,$1,$2);}
	|specifier_qualifier_list			{$$=AddTreeNode(1196,1,$1);}
	;

abstract_declarator
	:pointer					{$$=AddTreeNode(1197,1,$1);}
	|pointer direct_abstract_declarator		{$$=AddTreeNode(1198,2,$1,$2);}
	|direct_abstract_declarator			{$$=AddTreeNode(1199,1,$1);}
	;

direct_abstract_declarator
	:LP abstract_declarator RP									{$$=AddTreeNode(1200,3,AddTreeLeaf(1200,$1),$2,AddTreeLeaf(1200,$3));}
	|direct_abstract_declarator LB type_qualifier_list assignment_expression RB			{$$=AddTreeNode(1201,5,$1,AddTreeLeaf(1201,$2),$3,$4,AddTreeLeaf(1201,$5));}
	|LB type_qualifier_list assignment_expression RB						{$$=AddTreeNode(1202,4,AddTreeLeaf(1202,$1),$2,$3,AddTreeLeaf(1202,$4));}
	|direct_abstract_declarator LB assignment_expression RB						{$$=AddTreeNode(1203,4,$1,AddTreeLeaf(1203,$2),$3,AddTreeLeaf(1203,$4));}
	|direct_abstract_declarator LB type_qualifier_list RB						{$$=AddTreeNode(1204,4,$1,AddTreeLeaf(1204,$2),$3,AddTreeLeaf(1204,$4));}
	|direct_abstract_declarator LB RB								{$$=AddTreeNode(1205,3,$1,AddTreeLeaf(1205,$2),AddTreeLeaf(1205,$3));}
	|LB type_qualifier_list RB									{$$=AddTreeNode(1206,3,AddTreeLeaf(1206,$1),$2,AddTreeLeaf(1206,$3));}
	|LB assignment_expression RB									{$$=AddTreeNode(1207,3,AddTreeLeaf(1207,$1),$2,AddTreeLeaf(1207,$3));}
	|LB RB												{$$=AddTreeNode(1208,2,AddTreeLeaf(1208,$1),AddTreeLeaf(1208,$2));}
	|direct_abstract_declarator LB STATIC type_qualifier_list assignment_expression RB		{$$=AddTreeNode(1209,6,$1,AddTreeLeaf(1209,$2),AddTreeLeaf(1209,$3),$4,$5,AddTreeLeaf(1209,$6));}
	|LB STATIC type_qualifier_list assignment_expression RB						{$$=AddTreeNode(1210,5,AddTreeLeaf(1210,$1),AddTreeLeaf(1210,$2),$3,$4,AddTreeLeaf(1210,$5));}
	|direct_abstract_declarator LB STATIC assignment_expression RB					{$$=AddTreeNode(1211,5,$1,AddTreeLeaf(1211,$2),AddTreeLeaf(1211,$3),$4,AddTreeLeaf(1211,$5));}
	|LB STATIC assignment_expression RB								{$$=AddTreeNode(1212,4,AddTreeLeaf(1212,$1),AddTreeLeaf(1212,$2),$3,AddTreeLeaf(1212,$4));}
	|direct_abstract_declarator LB type_qualifier_list STATIC assignment_expression RB		{$$=AddTreeNode(1213,6,$1,AddTreeLeaf(1213,$2),$3,AddTreeLeaf(1213,$4),$5,AddTreeLeaf(1213,$6));}
	|LB type_qualifier_list STATIC assignment_expression RB						{$$=AddTreeNode(1214,5,AddTreeLeaf(1214,$1),$2,AddTreeLeaf(1214,$3),$4,AddTreeLeaf(1214,$5));}
	|direct_abstract_declarator LB MUL RB								{$$=AddTreeNode(1215,4,$1,AddTreeLeaf(1215,$2),AddTreeLeaf(1215,$3),AddTreeLeaf(1215,$4));}
	|LB MUL RB											{$$=AddTreeNode(1216,3,AddTreeLeaf(1216,$1),AddTreeLeaf(1216,$2),AddTreeLeaf(1216,$3));}
	|direct_abstract_declarator LP parameter_type_list RP						{$$=AddTreeNode(1217,4,$1,AddTreeLeaf(1217,$2),$3,AddTreeLeaf(1217,$4));}
	|LP parameter_type_list RP									{$$=AddTreeNode(1218,3,AddTreeLeaf(1218,$1),$2,AddTreeLeaf(1218,$3));}
	|direct_abstract_declarator LP RP								{$$=AddTreeNode(1219,3,$1,AddTreeLeaf(1219,$2),AddTreeLeaf(1219,$3));}
	|LP RP												{$$=AddTreeNode(1220,2,AddTreeLeaf(1220,$1),AddTreeLeaf(1220,$2));}
	;
/*
typedef_name
	:IDENTIFIER			{$$=AddTreeLeaf(1057,$1);}
	;
*/
initializer
	:assignment_expression		{$$=AddTreeNode(1221,1,$1);}
	|LC initializer_list RC		{$$=AddTreeNode(1222,3,AddTreeLeaf(1222,$1),$2,AddTreeLeaf(1222,$3));}
	|LC initializer_list CM RC	{$$=AddTreeNode(1223,4,AddTreeLeaf(1223,$1),$2,AddTreeLeaf(1223,$3),AddTreeLeaf(1223,$4));}
	;

initializer_list
	:designation initializer					{$$=AddTreeNode(1224,2,$1,$2);}
	|initializer							{$$=AddTreeNode(1225,1,$1);}
	|initializer_list CM designation initializer			{$$=AddTreeNode(1226,4,$1,AddTreeLeaf(1226,$2),$3,$4);}
	|initializer_list CM initializer				{$$=AddTreeNode(1227,3,$1,AddTreeLeaf(1227,$2),$3);}
	;

designation
	:designator_list ASN	{$$=AddTreeNode(1228,2,$1,AddTreeLeaf(1228,$2));}
	;

designator_list
	:designator				{$$=AddTreeNode(1229,1,$1);}
	|designator_list designator		{$$=AddTreeNode(1230,2,$1,$2);}
	;

designator
	:LB constant_expression RB		{$$=AddTreeNode(1231,3,AddTreeLeaf(1231,$1),$2,AddTreeLeaf(1231,$3));}
	|DOT IDENTIFIER				{$$=AddTreeNode(1232,2,AddTreeLeaf(1232,$1),AddTreeLeaf(1232,$2));}
	;

static_assert_declaration
	:_STATIC_ASSERT LP constant_expression CM STRING_LITERAL RP SM		{$$=AddTreeNode(1233,7,AddTreeLeaf(1233,$1),AddTreeLeaf(1233,$2),$3,AddTreeLeaf(1233,$4),AddTreeLeaf(1233,$5),AddTreeLeaf(1233,$6),AddTreeLeaf(1233,$7));}
	;

statement
	:labeled_statement			{$$=AddTreeNode(1234,1,$1);}
	|compound_statement			{$$=AddTreeNode(1235,1,$1);}
	|expression_statement			{$$=AddTreeNode(1236,1,$1);}
	|selection_statement			{$$=AddTreeNode(1237,1,$1);}
	|iteration_statement			{$$=AddTreeNode(1238,1,$1);}
	|jump_statement				{$$=AddTreeNode(1239,1,$1);}
	;

labeled_statement
	:identifier COLON statement			{$$=AddTreeNode(1240,3,$1,AddTreeLeaf(1240,$2),$3);}
	|CASE constant_expression COLON statement	{$$=AddTreeNode(1241,4,AddTreeLeaf(1241,$1),$2,AddTreeLeaf(1241,$3),$4);}
	|DEFAULT COLON statement			{$$=AddTreeNode(1242,3,AddTreeLeaf(1242,$1),AddTreeLeaf(1242,$2),$3);}
	;

compound_statement
	:LC block_item_list RC				{$$=AddTreeNode(1243,3,AddTreeLeaf(1243,$1),$2,AddTreeLeaf(1243,$3));}
	|LC RC						{$$=AddTreeNode(1244,2,AddTreeLeaf(1244,$1),AddTreeLeaf(1244,$2));}
	;

block_item_list
	:block_item					{$$=AddTreeNode(1245,1,$1);}
	|block_item_list block_item			{$$=AddTreeNode(1246,2,$1,$2);}
	;

block_item
	:declaration					{$$=AddTreeNode(1247,1,$1);}
	|statement					{$$=AddTreeNode(1248,1,$1);}
	;

expression_statement
	:expression SM					{$$=AddTreeNode(1249,2,$1,AddTreeLeaf(1249,$2));}
	|SM						{$$=AddTreeLeaf(1250,$1);}
	;

selection_statement
	:IF LP expression RP statement	%prec LOWER_THAN_ELSE	{$$=AddTreeNode(1251,5,AddTreeLeaf(1251,$1),AddTreeLeaf(1251,$2),$3,AddTreeLeaf(1251,$4),$5);}
	|IF LP expression RP statement ELSE statement		{$$=AddTreeNode(1252,7,AddTreeLeaf(1252,$1),AddTreeLeaf(1252,$2),$3,AddTreeLeaf(1252,$4),$5,AddTreeLeaf(1252,$6),$7);}
	|SWITCH LP expression RP statement			{$$=AddTreeNode(1253,5,AddTreeLeaf(1253,$1),AddTreeLeaf(1253,$2),$3,AddTreeLeaf(1253,$4),$5);}
	;

iteration_statement
	:WHILE LP expression RP statement					{$$=AddTreeNode(1254,5,AddTreeLeaf(1254,$1),AddTreeLeaf(1254,$2),$3,AddTreeLeaf(1254,$4),$5);}
	|DO statement WHILE LP expression RP SM					{$$=AddTreeNode(1255,7,AddTreeLeaf(1255,$1),$2,AddTreeLeaf(1255,$3),AddTreeLeaf(1255,$4),$5,AddTreeLeaf(1255,$6),AddTreeLeaf(1255,$7));}
	|FOR LP expression SM expression SM expression RP statement		{$$=AddTreeNode(1256,9,AddTreeLeaf(1256,$1),AddTreeLeaf(1256,$2),$3,AddTreeLeaf(1256,$4),$5,AddTreeLeaf(1256,$6),$7,AddTreeLeaf(1256,$8),$9);}
	|FOR LP SM expression SM expression RP statement			{$$=AddTreeNode(1257,8,AddTreeLeaf(1257,$1),AddTreeLeaf(1257,$2),AddTreeLeaf(1257,$3),$4,AddTreeLeaf(1257,$5),$6,AddTreeLeaf(1257,$7),$8);}
	|FOR LP expression SM SM expression RP statement			{$$=AddTreeNode(1258,8,AddTreeLeaf(1258,$1),AddTreeLeaf(1258,$2),$3,AddTreeLeaf(1258,$4),AddTreeLeaf(1258,$5),$6,AddTreeLeaf(1258,$7),$8);}
	|FOR LP expression SM expression SM RP statement			{$$=AddTreeNode(1259,8,AddTreeLeaf(1259,$1),AddTreeLeaf(1259,$2),$3,AddTreeLeaf(1259,$4),$5,AddTreeLeaf(1259,$6),AddTreeLeaf(1259,$7),$8);}
	|FOR LP SM SM expression RP statement					{$$=AddTreeNode(1260,7,AddTreeLeaf(1260,$1),AddTreeLeaf(1260,$2),AddTreeLeaf(1260,$3),AddTreeLeaf(1260,$4),$5,AddTreeLeaf(1260,$6),$7);}
	|FOR LP SM expression SM RP statement					{$$=AddTreeNode(1261,7,AddTreeLeaf(1261,$1),AddTreeLeaf(1261,$2),AddTreeLeaf(1261,$3),$4,AddTreeLeaf(1261,$5),AddTreeLeaf(1261,$6),$7);}
	|FOR LP expression SM SM RP statement					{$$=AddTreeNode(1262,7,AddTreeLeaf(1262,$1),AddTreeLeaf(1262,$2),$3,AddTreeLeaf(1262,$4),AddTreeLeaf(1262,$5),AddTreeLeaf(1262,$6),$7);}
	|FOR LP SM SM RP statement						{$$=AddTreeNode(1263,6,AddTreeLeaf(1263,$1),AddTreeLeaf(1263,$2),AddTreeLeaf(1263,$3),AddTreeLeaf(1263,$4),AddTreeLeaf(1263,$5),$6);}
	|FOR LP declaration expression SM expression RP statement		{$$=AddTreeNode(1264,8,AddTreeLeaf(1264,$1),AddTreeLeaf(1264,$2),$3,$4,AddTreeLeaf(1264,$5),$6,AddTreeLeaf(1264,$7),$8);}
	|FOR LP declaration SM expression RP statement				{$$=AddTreeNode(1265,7,AddTreeLeaf(1265,$1),AddTreeLeaf(1265,$2),$3,AddTreeLeaf(1265,$4),$5,AddTreeLeaf(1265,$6),$7);}
	|FOR LP declaration expression SM RP statement				{$$=AddTreeNode(1266,7,AddTreeLeaf(1266,$1),AddTreeLeaf(1266,$2),$3,$4,AddTreeLeaf(1266,$5),AddTreeLeaf(1266,$6),$7);}
	|FOR LP declaration SM RP statement					{$$=AddTreeNode(1267,6,AddTreeLeaf(1267,$1),AddTreeLeaf(1267,$2),$3,AddTreeLeaf(1267,$4),AddTreeLeaf(1267,$5),$6);}
	;

jump_statement
	:GOTO identifier SM		{$$=AddTreeNode(1268,3,AddTreeLeaf(1268,$1),$2,AddTreeLeaf(1268,$3));}
	|CONTINUE SM			{$$=AddTreeNode(1269,2,AddTreeLeaf(1269,$1),AddTreeLeaf(1269,$2));}
	|BREAK SM			{$$=AddTreeNode(1270,2,AddTreeLeaf(1270,$1),AddTreeLeaf(1270,$2));}
	|RETURN expression SM		{$$=AddTreeNode(1271,3,AddTreeLeaf(1271,$1),$2,AddTreeLeaf(1271,$3));}
	|RETURN SM			{$$=AddTreeNode(1272,2,AddTreeLeaf(1272,$1),AddTreeLeaf(1272,$2));}
	;

translation_unit
	:external_declaration				{$$=AddTreeNode(1273,1,$1);}
	|translation_unit external_declaration		{$$=AddTreeNode(1274,2,$1,$2);}
	;

external_declaration
	:function_definition				{$$=AddTreeNode(1275,1,$1);}
	|declaration					{$$=AddTreeNode(1276,1,$1);}
	;

function_definition
	:declaration_specifiers declarator declaration_list compound_statement	{$$=AddTreeNode(1277,4,$1,$2,$3,$4);}
	|declaration_specifiers declarator compound_statement			{$$=AddTreeNode(1278,3,$1,$2,$3);}
	;

declaration_list
	:declaration					{$$=AddTreeNode(1279,1,$1);}
	|declaration_list declaration			{$$=AddTreeNode(1280,2,$1,$2);}
	;

file
	:translation_unit				{$$=AddTreeNode(1281,1,$1);yytreeroot=$$;}
	;

%%





struct tree* AddTreeLeaf(int rule, struct token* tokenleaf)
{
	struct tree* ptr=(struct tree*)malloc(1*sizeof(struct tree));
	if (ptr==NULL)
		perror("Error occurs when allocating tree leaf memory!\n");

	ptr->prodrule=rule;
	ptr->nkids=0;
	ptr->kids=NULL;
	ptr->leaf=tokenleaf;
	
	return ptr;
}
struct tree* AddTreeNode(int rule, int kids, ...)
{
	int i;
	va_list node;
	struct tree* ptr=(struct tree*)malloc(1*sizeof(struct tree));
	if (ptr==NULL)
		perror("Error occurs when allocating tree leaf memory!\n");

	ptr->prodrule=rule;
	ptr->nkids=kids;
	ptr->leaf=NULL;
	ptr->kids=(struct tree**)malloc(kids*sizeof(struct tree*));
	if(ptr->kids==NULL)
		perror("Error occurs when allocating tree kids array memory!\n");
	
	va_start(node,kids);
	for(i=0;i<kids;i++)
	{
	ptr->kids[i]=va_arg(node,struct tree*);
	}
	va_end(node);

	return ptr;
}



void yyerror(char const *s)
{
    fprintf(stderr, "%s in file: %s line: %d\n", s,yyname,line_num);
}


