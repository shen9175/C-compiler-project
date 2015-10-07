sign				[\+\-]
digit	                	[0-9]
nonzero_digit			[1-9]
octal_digit   	        	[0-7]
hexadecimal_digit       	[a-fA-F0-9]
nondigit                       	[_a-zA-Z]
hex_quad			({hexadecimal_digit}{4})
universal_character_name	(\\u{hex_quad})|(\\U({hex_quad}{2}))

hexadecimal_escape_sequence	\\x({hexadecimal_digit}+)
octal_escape_sequence		\\({octal_digit}{1,3})
simple_escape_sequence		(\\')|(\\\")|(\\\?)|(\\\\)|(\\a)|(\\b)|(\\f)|(\\n)|(\\r)|(\\t)|(\\v)
escape_sequence			({simple_escape_sequence})|({octal_escape_sequence})|({hexadecimal_escape_sequence})|({universal_character_name})
c_char				([^'\\\n])|{escape_sequence}
c_char_sequence			({c_char}+)
digit_sequence			({digit}+)
hexadecimal_digit_sequence	({hexadecimal_digit}+)

s_char				([^\"\\\n)])|{escape_sequence}
s_char_sequence			({s_char}+)

q_char				[^\"\n]
q_char_sequence			({q_char}+)

h_char				[^>\n]
h_char_sequence			({h_char}+)

header_name			((<{h_char_sequence}>)|(\"{q_char_sequence}\"))

encoding_prefix			((u8)|[uUL])
hexadecimal_prefix		(0[xX])

unsigned_suffix			(u|U)
long_suffix			(l|L)
long_long_suffix		(ll|LL)
integer_suffix          	({unsigned_suffix}{long_suffix}?)|({unsigned_suffix}{long_long_suffix})|({long_suffix}{unsigned_suffix}?)|({long_long_suffix}{unsigned_suffix}?)
floating_suffix                 (f|F|l|L)

exponent_part			([Ee]{sign}?{digit_sequence})
binary_exponent_part		([Pp]{sign}?{digit_sequence})



hexadecimal_constant		({hexadecimal_prefix}{hexadecimal_digit}+)
octal_constant			(0{octal_digit}*)
decimal_constant		({nonzero_digit}{digit}*)

fractional_constant		({digit_sequence}?\.{digit_sequence})|({digit_sequence}\.)
hexadecimal_fractional_constant	({hexadecimal_digit_sequence}?\.{hexadecimal_digit_sequence})|({hexadecimal_digit_sequence}\.)



identifier_nondigit		({nondigit}|{universal_character_name})

pp_number			((\.)?{digit}({digit}|{identifier_nondigit}|([PpEe]{sign})|\.)*)

whitespace			([ \t\f]+)

%{


#include "cgram.tab.h"
#include "parsetree.h" 


char* yyname;
struct token yytoken;

struct token* AddToken(struct token p);

int add_token_identifier(int r);
int add_token_keywords(int r);
int add_token_punctuators(int r);

int add_token_hexadecimal();
int add_token_octal();
int add_token_decimal();
int add_token_decimal_floating();
int add_token_hexadecimal_floating();
int add_token_character();
int add_token_string();
void line_directive();
int line_num = 1;


%}





%%

(\/\*([^\*]|[\r\n]|(\*+([^\*\/]|[\r\n])))*\*+\/)|(\/\/.*) { }
{whitespace}                				  { }
^#{whitespace}{digit}+{whitespace}.*\n			  { line_directive(); }
\n                      				  { line_num++; }



"auto"                  { return(add_token_keywords(AUTO)); }
"break"                 { return(add_token_keywords(BREAK)); }
"case"                  { return(add_token_keywords(CASE)); }
"char"                  { return(add_token_keywords(CHAR)); }
"const"                 { return(add_token_keywords(CONST)); }
"continue"              { return(add_token_keywords(CONTINUE)); }
"default"               { return(add_token_keywords(DEFAULT)); }
"do"                    { return(add_token_keywords(DO)); }
"double"                { return(add_token_keywords(DOUBLE)); }
"else"                  { return(add_token_keywords(ELSE)); }
"enum"                  { return(add_token_keywords(ENUM)); }
"extern"                { return(add_token_keywords(EXTERN)); }
"float"                 { return(add_token_keywords(FLOAT)); }
"for"                   { return(add_token_keywords(FOR)); }
"goto"                  { return(add_token_keywords(GOTO)); }
"if"                    { return(add_token_keywords(IF)); }
"inline"		{ return(add_token_keywords(INLINE)); }
"int"                   { return(add_token_keywords(INT)); }
"long"                  { return(add_token_keywords(LONG)); }
"register"              { return(add_token_keywords(REGISTER)); }
"restrict"		{ return(add_token_keywords(RESTRICT)); }
"return"                { return(add_token_keywords(RETURN)); }
"short"                 { return(add_token_keywords(SHORT)); }
"signed"                { return(add_token_keywords(SIGNED)); }
"sizeof"                { return(add_token_keywords(SIZEOF)); }
"static"                { return(add_token_keywords(STATIC)); }
"struct"                { return(add_token_keywords(STRUCT)); }
"switch"                { return(add_token_keywords(SWITCH)); }
"typedef"               { return(add_token_keywords(TYPEDEF)); }
"union"                 { return(add_token_keywords(UNION)); }
"unsigned"              { return(add_token_keywords(UNSIGNED)); }
"void"                  { return(add_token_keywords(VOID)); }
"volatile"              { return(add_token_keywords(VOLATILE)); }
"while"                 { return(add_token_keywords(WHILE)); }
"_Alignas"		{ return(add_token_keywords(_ALIGNAS)); }
"_Alignof"		{ return(add_token_keywords(_ALIGNOF)); }
"_Atomic"		{ return(add_token_keywords(_ATOMIC)); }
"_Bool"			{ return(add_token_keywords(_BOOL)); }
"_Complex"		{ return(add_token_keywords(_COMPLEX)); }
"_Generic"		{ return(add_token_keywords(_GENERIC)); }
"_Imaginary"		{ return(add_token_keywords(_IMAGINARY)); }
"_Noreturn"		{ return(add_token_keywords(_NORETURN)); }
"_Static_assert"	{ return(add_token_keywords(_STATIC_ASSERT)); }
"_Thread_local"		{ return(add_token_keywords(_THREAD_LOCAL)); }
"table"			{ return(add_token_keywords(TABLE)); }



{identifier_nondigit}({identifier_nondigit}|{digit})*			           				{ return(add_token_identifier(IDENTIFIER));}
{hexadecimal_constant}+{integer_suffix}?									{ return(add_token_hexadecimal()); }
{octal_constant}+{integer_suffix}?              								{ return(add_token_octal()); }
{decimal_constant}+{integer_suffix}?            								{ return(add_token_decimal());}
('{c_char_sequence}')|(L'{c_char_sequence}')|(u'{c_char_sequence}')|(U'{c_char_sequence}')      		{ return(add_token_character());}
({fractional_constant}{exponent_part}?{floating_suffix}?)|({digit_sequence}{exponent_part}{floating_suffix}?)	{ return(add_token_decimal_floating());}
({hexadecimal_prefix}{hexadecimal_fractional_constant}{binary_exponent_part}{floating_suffix}?)|({hexadecimal_prefix}{hexadecimal_digit_sequence}{binary_exponent_part}{floating_suffix}?)	{ return(add_token_hexadecimal_floating());}
\"(\\.|[^\\"])*\" 			 									{ return(add_token_string());}


">>="                   { return(add_token_punctuators(SRASN)); }
"<<="                   { return(add_token_punctuators(SLASN)); }
"+="                    { return(add_token_punctuators(PLASN)); }
"-="                    { return(add_token_punctuators(MIASN)); }
"*="                    { return(add_token_punctuators(MUASN)); }
"/="                    { return(add_token_punctuators(DIASN)); }
"%="                    { return(add_token_punctuators(MOASN)); }
"&="                    { return(add_token_punctuators(ANASN)); }
"^="                    { return(add_token_punctuators(ERASN)); }
"|="                    { return(add_token_punctuators(ORASN)); }
">>"                    { return(add_token_punctuators(SHR)); }
"<<"                    { return(add_token_punctuators(SHL)); }
"++"                    { return(add_token_punctuators(INCOP)); }
"--"                    { return(add_token_punctuators(DECOP)); }
"->"                    { return(add_token_punctuators(FOLLOW)); }
"&&"                    { return(add_token_punctuators(ANDAND)); }
"||"                    { return(add_token_punctuators(OROR)); }
"<="                    { return(add_token_punctuators(LE)); }
">="                    { return(add_token_punctuators(GE)); }
"=="                    { return(add_token_punctuators(EQ)); }
"!="                    { return(add_token_punctuators(NE)); }
";"                     { return(add_token_punctuators(SM)); }
"{"                     { return(add_token_punctuators(LC)); }
"}"                     { return(add_token_punctuators(RC)); }
","                     { return(add_token_punctuators(CM)); }
":"                     { return(add_token_punctuators(COLON)); }
"="                     { return(add_token_punctuators(ASN)); }
"("                     { return(add_token_punctuators(LP)); }
")"                     { return(add_token_punctuators(RP)); }
"["                     { return(add_token_punctuators(LB)); }
"]"                     { return(add_token_punctuators(RB)); }
"."                     { return(add_token_punctuators(DOT)); }
"&"                     { return(add_token_punctuators(AND)); }
"!"                     { return(add_token_punctuators(BANG)); }
"~"                     { return(add_token_punctuators(NOT)); }
"-"                     { return(add_token_punctuators(MINUS)); }
"+"                     { return(add_token_punctuators(PLUS)); }
"*"                     { return(add_token_punctuators(MUL)); }
"/"                     { return(add_token_punctuators(DIV)); }
"%"                     { return(add_token_punctuators(MOD)); }
"<"                     { return(add_token_punctuators(LT)); }
">"                     { return(add_token_punctuators(GT)); }
"^"                     { return(add_token_punctuators(ER)); }
"|"                     { return(add_token_punctuators(OR)); }
"?"                     { return(add_token_punctuators(QUEST)); }
"..."                   { return(add_token_punctuators(ELIPSIS)); }
"#"			{ return(add_token_punctuators(POUND)); }
"##"			{ return(add_token_punctuators(DPOUND)); }
"<:"			{ return(add_token_punctuators(LB)); }
":>"			{ return(add_token_punctuators(RB)); }
"<%"			{ return(add_token_punctuators(LC)); }
"%>"			{ return(add_token_punctuators(RC)); }
"%:"			{ return(add_token_punctuators(POUND)); }
"%:%:"			{ return(add_token_punctuators(DPOUND)); }
.			{ fprintf(stderr,"file: %s line %d: Stray %s in program.\nyylex returned 1\n",yyname,line_num,yytext);exit(1);}

%%

struct token* AddToken(struct token p)
{
	struct token* t;

	if((t=(struct token*)malloc(1*sizeof(struct token)))==NULL)
		perror("Error occurs during allocating new token memory in the tokenlist node when adding new token into tokenlist node!\n");

	t->category=p.category;


	t->text=strdup(p.text);

	t->lineno=p.lineno;

	t->filename=strdup(p.filename);
	
	t->ival=p.ival;

	if(p.sval!=NULL)
	t->sval=strdup(p.sval);
	else
		t->sval=NULL;

	return t;
}



int add_token_identifier(int r)
{
yytoken.category=r;
yytoken.text=strdup(yytext);
yytoken.lineno=line_num;
yytoken.filename=strdup(yyname);
yytoken.ival=0;
yytoken.sval=NULL;
yylval.tokptr=AddToken(yytoken);

free(yytoken.text);
yytoken.text=NULL;
free(yytoken.filename);
yytoken.filename=NULL;
free(yytoken.sval);
yytoken.sval=NULL;

return r;
}

int add_token_keywords(int r)
{
yytoken.category=r;
yytoken.text=strdup(yytext);
yytoken.lineno=line_num;
yytoken.filename=strdup(yyname);
yytoken.ival=0;
yytoken.sval=NULL;
yylval.tokptr=AddToken(yytoken);

free(yytoken.text);
yytoken.text=NULL;
free(yytoken.filename);
yytoken.filename=NULL;
free(yytoken.sval);
yytoken.sval=NULL;

return r;
}

int add_token_punctuators(int r)
{
yytoken.category=r;
yytoken.text=strdup(yytext);
yytoken.lineno=line_num;
yytoken.filename=strdup(yyname);
yytoken.ival=0;
yytoken.sval=NULL;
yylval.tokptr=AddToken(yytoken);

free(yytoken.text);
yytoken.text=NULL;
free(yytoken.filename);
yytoken.filename=NULL;
free(yytoken.sval);
yytoken.sval=NULL;

return r;
}

int add_token_hexadecimal()
{
yytoken.category=ICON;
yytoken.text=strdup(yytext);
yytoken.lineno=line_num;
yytoken.filename=strdup(yyname);
int i,j=0,number=0;
for(i=strlen(yytext)-1;i>=2;i--)
{
 if(yytext[i]!='u'&&yytext[i]!='U'||yytext[i]!='L'||yytext[i]!='l') //skip optinal integer suffix
 {
  if(yytext[i]>=48 && yytext[i]<=57) //[0-9]
	number=number+(yytext[i]-48)*pow(16,j);

  else if(yytext[i]=='A'||yytext[i]=='a')
	number=number+10*pow(16,j);
  else if(yytext[i]=='B'||yytext[i]=='b')
	number=number+11*pow(16,j);
  else if(yytext[i]=='C'||yytext[i]=='c')
	number=number+12*pow(16,j);
  else if(yytext[i]=='D'||yytext[i]=='d')
	number=number+13*pow(16,j);
  else if(yytext[i]=='E'||yytext[i]=='e')
	number=number+14*pow(16,j);
  else if(yytext[i]=='F'||yytext[i]=='f')
	number=number+15*pow(16,j);
  else
	perror("Wrong hexdecimal number!\n");
  j++;
 }
}
yytoken.ival=number;
yytoken.sval=NULL;
yylval.tokptr=AddToken(yytoken);

free(yytoken.text);
yytoken.text=NULL;
free(yytoken.filename);
yytoken.filename=NULL;
free(yytoken.sval);
yytoken.sval=NULL;

return ICON;
}

int add_token_octal()
{
yytoken.category=ICON;
yytoken.text=strdup(yytext);
yytoken.lineno=line_num;
yytoken.filename=strdup(yyname);
int i,j=0,number=0;

 for(i=strlen(yytext)-1;i>=1;i--)  //if yytext is "0" then strlen(yytext)=1, then i=1-1=0, 0>=1 is not true so the for loop does not execute, so number=0
 {
   if(yytext[i]!='u'&&yytext[i]!='U'||yytext[i]!='L'||yytext[i]!='l') //skip optinal integer suffix
   {
     if(yytext[i]>=48 && yytext[i]<=55) //[0-7]
	   number=number+(yytext[i]-48)*pow(8,j);
    else
   	   perror("Wrong octal number!\n");
    j++;
   }
 }


yytoken.ival=number;
yytoken.sval=NULL;
yylval.tokptr=AddToken(yytoken);

free(yytoken.text);
yytoken.text=NULL;
free(yytoken.filename);
yytoken.filename=NULL;
free(yytoken.sval);
yytoken.sval=NULL;

return ICON;
}

int add_token_decimal()
{
yytoken.category=ICON;
yytoken.text=strdup(yytext);
yytoken.lineno=line_num;
yytoken.filename=strdup(yyname);
yytoken.ival=atoi(yytext);
yytoken.sval=NULL;
yylval.tokptr=AddToken(yytoken);

free(yytoken.text);
yytoken.text=NULL;
free(yytoken.filename);
yytoken.filename=NULL;
free(yytoken.sval);
yytoken.sval=NULL;
return ICON;
}

int add_token_character()
{
yytoken.category=CCON;
yytoken.text=strdup(yytext);
yytoken.lineno=line_num;
yytoken.filename=strdup(yyname);

int i=0;
while(yytext[i]!=39)
{i++;}
char *buf=strdup(yytext);
char *start = &buf[i+1];
int j=strlen(buf)-1;
while(buf[j]!=39)
{j--;}
buf[j]='\0';
 
if(strlen(start)==1)//not escape sequence
{
 if(start[0]!=92)//92->\ backslash
	yytoken.ival=start[0];
 else  {
	printf("Line: %d The character can't be single backslash!\n",line_num);
	yytoken.ival=0;
	}
}
else if(strlen(start)==0)
	yytoken.ival=0;
else if(strlen(start)>1&&start[0]==92)
	{
	  if(start[1]=='x'&&(strlen(start)==3||strlen(start)==4))
	   {
		if(strlen(start)==3)
		{
			if(isxdigit(start[2]))
				yytoken.ival=start[2]-'0';
			else{
				printf("Line: %d The character is not hexadecimal number!\n",line_num);
				yytoken.ival=0;
		    	}
		}
		else if(strlen(start)==4)
			{
				if(isxdigit(start[2])&&isxdigit(start[3]))
					{
						char hex[3];
						hex[0]=start[2];
						hex[1]=start[3];
						hex[2]='\0';
						long int hexi=strtol(hex,NULL,16);
						yytoken.ival=(char)hexi;
					}
				else{
					printf("Line: %d The character is not hexadecimal number!\n",line_num);
					yytoken.ival=0;
				    }
			}
		else ;
	    }
	    else if((start[1]-'0')>=0&&(start[1]-'0')<=7)
		{
		  if(strlen(start)==2)
			yytoken.ival=start[1];
		  else if(strlen(start)==3)
			{
			if((start[2]-'0')>=0&&(start[2]-'0')<=7)
			yytoken.ival=(start[1]-'0')*8+(start[2]-'0');
			else{printf("Line: %d The character is not hexadecimal number!\n",line_num);yytoken.ival=0;}
			}
		  else if((strlen(start)==4)&&((start[2]-'0')>=0&&(start[2]-'0')<=7)&&((start[3]-'0')>=0&&(start[3]-'0')<=7))
			{
				if((start[1]-'0')*64+(start[2]-'0')*8+(start[2]-'0')>127)
					{printf("Line: %d The character is not a 8bit character!\n",line_num);yytoken.ival=0;}
				else
					yytoken.ival=(start[1]-'0')*64+(start[2]-'0')*8+(start[2]-'0');
			}
		  else
			{printf("Line: %d The character is not a valid octal escape sequence character!\n",line_num);yytoken.ival=0;}
		}
	   else if(start[1]=='u'&&strlen(start)==6&&isxdigit(start[2])&&isxdigit(start[3])&&isxdigit(start[4])&&isxdigit(start[5]))
		{
			char hex[5];
			hex[0]=start[2];
			hex[1]=start[3];
			hex[2]=start[4];
			hex[3]=start[5];
			hex[4]='\0';
			long int hexi=strtol(hex,NULL,16);
			if(hexi==0x24||hexi==0x40||hexi==0x60) //   only do 8bit character the rest of them will be ignored ||(hexi>=0x00A0 && hexi<0xD800)||(hexi>0xDFFF&&hexi<0xFFFF)
				yytoken.ival=(char)hexi;
			else {printf("Line: %d The character is not a valid 8bit universal character!\n",line_num);yytoken.ival=0;}
		}
	    else if(start[1]=='U'&&strlen(start)==10&&isxdigit(start[2])&&isxdigit(start[3])&&isxdigit(start[4])&&isxdigit(start[5])&&isxdigit(start[6])&&isxdigit(start[7])&&isxdigit(start[8])&&isxdigit(start[9]))
		{
			char hex[9];
			hex[0]=start[2];
			hex[1]=start[3];
			hex[2]=start[4];
			hex[3]=start[5];
			hex[4]=start[6];
			hex[5]=start[7];
			hex[6]=start[8];
			hex[7]=start[9];
			hex[8]='\0';
			long int hexi=strtol(hex,NULL,16);
			if(hexi==0x24||hexi==0x40||hexi==0x60) //   only do 8bit character the rest of them will be ignored ||(hexi>=0x00A0 && hexi<0xD800)||(hexi>0xDFFF&&hexi<0xFFFF)
				yytoken.ival=(char)hexi;
			else {printf("Line: %d The character is not a valid 8bit universal character!\n",line_num);yytoken.ival=0;}
		}
	    else if(start[1]==39&&strlen(start)==2)//'
			yytoken.ival=39;
	    else if(start[1]==34&&strlen(start)==2)//"
			yytoken.ival=34;
	    else if(start[1]=='?'&&strlen(start)==2)
			yytoken.ival='?';
	    else if(start[1]==92&&strlen(start)==2)//"\"
			yytoken.ival=92;
	    else if(start[1]=='a'&&strlen(start)==2)
			yytoken.ival='\a';
	    else if(start[1]=='b'&&strlen(start)==2)
			yytoken.ival='\b';
	    else if(start[1]=='f'&&strlen(start)==2)
			yytoken.ival='\f';
	    else if(start[1]=='n'&&strlen(start)==2)
			yytoken.ival='\n';
	    else if(start[1]=='r'&&strlen(start)==2)
			yytoken.ival='\r';
	    else if(start[1]=='t'&&strlen(start)==2)
			yytoken.ival='\t';
	    else if(start[1]=='v'&&strlen(start)==2)
			yytoken.ival='\v';
	    else {printf("Line: %d The character is not a valid escape sequence character!\n",line_num);yytoken.ival=0;}
	}
free(buf);
buf=NULL;

yytoken.sval=NULL;
yylval.tokptr=AddToken(yytoken);

free(yytoken.text);
yytoken.text=NULL;
free(yytoken.filename);
yytoken.filename=NULL;
free(yytoken.sval);
yytoken.sval=NULL;

return CCON;
}

int add_token_decimal_floating()
{
yytoken.category=FCON;
yytoken.text=strdup(yytext);
yytoken.lineno=line_num;
yytoken.filename=strdup(yyname);

yytoken.ival=0;
yytoken.sval=NULL;
yylval.tokptr=AddToken(yytoken);

free(yytoken.text);
yytoken.text=NULL;
free(yytoken.filename);
yytoken.filename=NULL;
free(yytoken.sval);
yytoken.sval=NULL;

return FCON;
}


int add_token_hexadecimal_floating()
{
yytoken.category=FCON;
yytoken.text=strdup(yytext);
yytoken.lineno=line_num;
yytoken.filename=strdup(yyname);

yytoken.ival=0;
yytoken.sval=NULL;
yylval.tokptr=AddToken(yytoken);

free(yytoken.text);
yytoken.text=NULL;
free(yytoken.filename);
yytoken.filename=NULL;
free(yytoken.sval);
yytoken.sval=NULL;

return FCON;
}

int add_token_string()
{
yytoken.category=STRING_LITERAL;
yytoken.text=strdup(yytext);
yytoken.lineno=line_num;
yytoken.filename=strdup(yyname);


if(yytext[0]!='"') //remove wide character prefix, this assignment only deal with 8bit character.
{
	if(yytext[0]=='u'&&yytext[1]=='8')
		yytext=yytext+2;
	else if(yytext[0]=='U'||yytext[0]=='u'||yytext[0]=='L')
		yytext++;
	else ;
}

char* temp;
if((temp=(char*)malloc((strlen(yytext)-2+1)*sizeof(char)))==NULL) //at least equal to yytext besides two double quote ""
	perror("Error during allocating memory for parsing string!\n");
int i,j;
i=0;
int flag=0; //this flag if it is 1 means, last character is just escape character, then we have do some specials.

//the kernal big unescape rountine of install string function.
for(j=1;j<strlen(yytext)-1-1;j++) 
{
	if(yytext[j]=='\\')
	{
		switch(yytext[j+1])
		{
			case '\'': {temp[i]='\'';if((j+1)==(strlen(yytext)-2)) flag=1;j++;}break;
			case '\"': {temp[i]='\"';if((j+1)==(strlen(yytext)-2)) flag=1;j++;}break;
			case '\?': {temp[i]='\?';if((j+1)==(strlen(yytext)-2)) flag=1;j++;}break;
			case '\\': {temp[i]='\\';if((j+1)==(strlen(yytext)-2)) flag=1;j++;}break;
			case 'a':  {temp[i]='\a';if((j+1)==(strlen(yytext)-2)) flag=1;j++;}break;
			case 'b':  {temp[i]='\b';if((j+1)==(strlen(yytext)-2)) flag=1;j++;}break;
			case 'f':  {temp[i]='\f';if((j+1)==(strlen(yytext)-2)) flag=1;j++;}break;
			case 'n':  {temp[i]='\n';if((j+1)==(strlen(yytext)-2)) flag=1;j++;}break;
			case 'r':  {temp[i]='\r';if((j+1)==(strlen(yytext)-2)) flag=1;j++;}break;
			case 't':  {temp[i]='\t';if((j+1)==(strlen(yytext)-2)) flag=1;j++;}break;
			case 'v':  {temp[i]='\v';if((j+1)==(strlen(yytext)-2)) flag=1;j++;}break;	//things above are undoing simple escape sequences
			case 'u':	//for Universal Character set (only deal 3 character since only these 3 are 8bit character)
				{
					if( (j+5)<=strlen(yytext)-2)
						{
							if( isxdigit(yytext[j+2])&&isxdigit(yytext[j+3])&&isxdigit(yytext[j+4])&&isxdigit(yytext[j+5]))
								{
									char hex[5];
									hex[0]=yytext[j+2];
									hex[1]=yytext[j+3];
									hex[2]=yytext[j+4];
									hex[3]=yytext[j+5];
									hex[4]='\0';
									long int hexi=strtol(hex,NULL,16);
									if(hexi==0x24||hexi==0x40||hexi==0x60) //   only do 8bit character the rest of them will be ignored ||(hexi>=0x00A0 && hexi<0xD800)||(hexi>0xDFFF&&hexi<0xFFFF)
									{
										temp[i]=(char)hexi;
										if((j+5)==strlen(yytext)-2)
											flag=1;
										j=j+5;

									}
									else
										temp[i]=yytext[j];//copy the exactly same character from input
								}
								else
									temp[i]=yytext[j];//copy the exactly same character from input
						}
						else
						temp[i]=yytext[j];//copy the exactly same character from input
				}
				break;

			case 'U':	//for Universal Character set (only deal 3 character since only these 3 are 8bit character)
				{
					if( (j+9)<=strlen(yytext)-2)
						{
							if( isxdigit(yytext[j+2])&&isxdigit(yytext[j+3])&&isxdigit(yytext[j+4])&&isxdigit(yytext[j+5])&&isxdigit(yytext[j+6])&&isxdigit(yytext[j+7])&&isxdigit(yytext[j+8])&&isxdigit(yytext[j+9]))
								{
									char hex[9];
									hex[0]=yytext[j+2];
									hex[1]=yytext[j+3];
									hex[2]=yytext[j+4];
									hex[3]=yytext[j+5];
									hex[4]=yytext[j+6];
									hex[5]=yytext[j+7];
									hex[6]=yytext[j+8];
									hex[7]=yytext[j+9];
									hex[8]='\0';
									long int hexi=strtol(hex,NULL,16);
									if(hexi==0x24||hexi==0x40||hexi==0x60) //   only do 8bit character the rest of them will be ignored ||(hexi>=0x00A0 && hexi<0xD800)||(hexi>0xDFFF&&hexi<0xFFFF)
									{
										temp[i]=(char)hexi;
										if((j+9)==strlen(yytext)-2)
											flag=1;
										j=j+9;

									}
									else
										temp[i]=yytext[j];//copy the exactly same character from input
								}
								else
									temp[i]=yytext[j];//copy the exactly same character from input
						}
						else
						temp[i]=yytext[j];//copy the exactly same character from input
				}
				break;

			case 'x':	//unescape 8bit hexadecimal escape sequence
				{
				  if((j+3)<=(strlen(yytext)-2) && isxdigit(yytext[j+2]) && isxdigit(yytext[j+3]))//hexadecimal escape sequence: two hex digits
					{
						char hex[3];
						hex[0]=yytext[j+2];
						hex[1]=yytext[j+3];
						hex[2]='\0';
						long int hexi=strtol(hex,NULL,16);

						if(hexi<=127)
							{
								temp[i]=(char)hexi;
								if((j+3)==(strlen(yytext)-2)) //the last character is just at the end of string(besides ")
									flag=1;
								j=j+3;
								
									
							}
						else
							{
								hex[1]='\0';//use one hex digit
								hexi=strtol(hex,NULL,16);
								temp[i]=(char)hexi;
								j=j+2;
							}
					}

				    else if((j+2)<=(strlen(yytext)-2) && isxdigit(yytext[j+2]))//hexadecimal escape sequence: one hex digit at the end
						{
							char hex[2];
							hex[0]=yytext[j+2];
							hex[1]='\0';
							long int hexi=strtol(hex,NULL,16);
							temp[i]=(char)hexi;
							if((j+2)==(strlen(yytext)-2))
								flag=1;
							j=j+2;
						}

				    else 
					 temp[i]=yytext[j];//copy the exactly same character from input
				
				} break;

			default:	
				{   
					//unescape 8bit ocatal escape sequence
 
					if(  (  (j+3)<=(strlen(yytext)-2) )  && (((yytext[j+1]-'0')>=0) && ((yytext[j+1]-'0')<=7))  )
						{
						   if(((yytext[j+2]-'0')>=0)   && ((yytext[j+2]-'0')<=7))
						    { 
							if(((yytext[j+3]-'0')>=0) && ((yytext[j+3]-'0')<=7))
						        {
								int oct=(yytext[j+1]-'0')*64+(yytext[j+2]-'0')*8+(yytext[j+3]-'0');

								if(oct>127)// the third digit is not belong to octal escape sequence
									{
							  			temp[i]=(yytext[j+1]-'0')*8+(yytext[j+2]-'0');
							  			j=j+2;
									}
								else
									{
							  			temp[i]=oct;
										if((j+3)==(strlen(yytext)-2))
											flag=1;
										j=j+3;							  				
									}
						         }
						       else
							  {
							  temp[i]=(yytext[j+1]-'0')*8+(yytext[j+2]-'0');
							  j=j+2;
							   }
						      }
						   else
						      {
							temp[i]=(yytext[j+1]-'0');
							j++;
						      }
						}
		
					 else if((j+2)<=(strlen(yytext)-2) && (  ((yytext[j+1]-'0')>=0) && ((yytext[j+1]-'0')<=7)) )
						{
							if( (((yytext[j+2]-'0')>=0) && ((yytext[j+2]-'0')<=7)))
							  {
							  	temp[i]=(yytext[j+1]-'0')*8+(yytext[j+2]-'0');
							   	if((j+2)==(strlen(yytext)-2))
									flag=1;
								j=j+2;
							  }
							 else
							  {
								temp[i]=(yytext[j+1]-'0');
								j++;
							  }
						}
					else if((j+1)<=(strlen(yytext)-2)&& (  ((yytext[j+1]-'0')>=0) && ((yytext[j+1]-'0')<=7)))
						{
								temp[i]=(yytext[j+1]-'0');
								if((j+1)==(strlen(yytext)-2))
									flag=1;
								j++;
						}
					else // deal with  incorrect escape sequence-->just copy as it is
						temp[i]=yytext[j];
				}break;				
		}
	}
	else 
	{
		temp[i]=yytext[j];	//copy normal characters
	}

	i++;
}

//deal some trick things on the edge of the string
if(flag==0)
{
temp[i]=yytext[j];
temp[i+1]='\0';
}
else
{
 temp[i]='\0';
}

yytoken.sval=strdup(temp);
free(temp);
temp=NULL;
yytoken.ival=0;

yylval.tokptr=AddToken(yytoken);

free(yytoken.text);
yytoken.text=NULL;
free(yytoken.filename);
yytoken.filename=NULL;
free(yytoken.sval);
yytoken.sval=NULL;

return STRING_LITERAL;
}

int yywrap()
{
   return 1;
}



void line_directive()
{
  char *NumSet   = "0123456789";
  char *buf      = strdup(yytext);
  char *line_ptr = strpbrk(buf, NumSet);
  char *file_ptr = strchr(line_ptr, '"')+1;  /* Start of file-name */


  line_num = atoi(line_ptr);

  if(file_ptr != 0) 
   {
     char *file_end = strrchr(file_ptr, '"');    /* end */
     char *slash;

     *(file_end) = 0;  /* remove quote from string */


     free(yyname);
     yyname=NULL;
     yyname = strdup(file_ptr); /* duplicate the string */    
   }
  free(buf);
}

