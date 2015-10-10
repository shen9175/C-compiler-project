#include <unistd.h>
#include "symtab.h"

FILE* yyin;
char* yyname;
struct tree* yytreeroot;
int newtempnumber;
int newlabelnumber;
int treeprint(struct tree *t, int depth);
int deletetree(struct tree* t);
int deletetoken(struct token* p);
void init_humanreadable();
char* humanreadable[282];

SymbolTable globals;
SymbolTable current;
int semantic_error;
int StrOffset;
int main(int argc, char *argv[])
{ 

   init_humanreadable();
   initial_types();
   int i=1;
   if (argc == 1)
      fprintf(stdout,"Please enter at least one input file!\n");
   else
     {
	int j,which,havec=0;
	for(j=0;j<argc;j++)
	{
		if(strcmp(argv[j],"-c")==0)//equal
			{which=j;havec=1;}
	}	
      while (--argc > 0)
	{
	if(which!=i)
	{
         if ((yyin = fopen(argv[i], "r")) == NULL)
	    {
            fprintf(stderr,"ct: can't open %s\n", *argv);
            return 1;
            }
         else 
	    {
		semantic_error=0;
		yyname=strdup(argv[i]);
	    	int code;
   		code = yyparse();
		if(code!=0)
			{
			code=2;
   			fprintf(stdout,"yyparse returned %d\n", code);
			}
		if(code==0)
			{
			//treeprint(yytreeroot,0);
			current=globals;
			populatesymbols(yytreeroot);
			typechecking(yytreeroot);
			if(semantic_error>0)
				fprintf(stderr,"semantic anlysis returned 3\n");
			else
				{
				   StrOffset=0;
				   newtempnumber=0;
				   newlabelnumber=0;
				//fprintf(stdout,"semantic anlysis returned 0\n");
				codegen(yytreeroot);
				char* output=strdup(yyname);
				output[strlen(yyname)-1]='s';
				output[strlen(yyname)]='\0';
				if((yyout=fopen(output,"w"))==NULL)
					{
						fprintf(stderr,"ct: can't create %s\n",output);
						return 1;
					}
				finalcode(yytreeroot);
				fclose(yyout);
				/* the following code is use gcc to compile the self-compiler generated assembly to binary
				you can disable it if you just want to see the assembly
				char bf[100];
				if(havec==1)//not specify the output name
					{
					sprintf(bf,"gcc -c %s",output);
					system(bf);
					//execlp("","gcc","-c",output,NULL);
					}
				else	{//specify the output file
					char* execu=strdup(output);
					execu[strlen(execu)-2]='\0';
					sprintf(bf,"gcc -o %s %s",execu,output);
					system(bf);
					//execlp("","gcc",output,NULL);
					}
					*/
				}
			}
		deletetree(yytreeroot);
            	fclose(yyin);
		free(yyname);
		yyname=NULL;
		yytreeroot=NULL;

            }//else

	  }//if
	i++;
	}//while

     }//else
   return 0;
}

int treeprint(struct tree *t, int depth)
{

	if(t->nkids!=0)
	{
		  int i;
		  printf("%*s (%d)%s: %d kids\n", depth*2, " ",t->prodrule,humanreadable[t->prodrule-1000], t->nkids);  
			for(i=0; i<t->nkids; i++)
			 	treeprint(t->kids[i], depth+1);
	}
	else 
	{
		printf("%*s (%d)%s: %d kids\n", depth*2, " ",t->prodrule,humanreadable[t->prodrule-1000], t->nkids); 
		if(t->leaf!=NULL)
			printf("%*s token: %d: %s\n", depth*2, " ", t->leaf->category, t->leaf->text);  
	}

	
	return 0;
}

int deletetree(struct tree* t)
{
	if(t->nkids!=0)
	{
		  int i=0;  
		  for(i=0; i<t->nkids; i++)
			{
		    	deletetree(t->kids[i]);
		  	t->kids[i]=NULL;
			}


		
	}
	else
	{
		if(t->leaf!=NULL)
			deletetoken(t->leaf);
		t->leaf=NULL;

	}
	
	free(t);

	return 0;
}

int deletetoken(struct token* p)
{
	free(p->text);
	p->text=NULL;
	free(p->filename);
	p->filename=NULL;
	free(p->sval);
	p->sval=NULL;
	free(p);

	return 0;
}

void init_humanreadable()
{
humanreadable[0]="identifier";
humanreadable[1]="identifier";

humanreadable[2]="constant";
humanreadable[3]="constant";
humanreadable[4]="constant";
humanreadable[5]="constant";

humanreadable[6]="primary_expression";
humanreadable[7]="primary_expression";
humanreadable[8]="primary_expression";
humanreadable[9]="primary_expression";
humanreadable[10]="primary_expression";

humanreadable[11]="generic_selection";
humanreadable[12]="generic_assoc_list";
humanreadable[13]="generic_assoc_list";

humanreadable[14]="generic_association";
humanreadable[15]="generic_association";

humanreadable[16]="postfix_expression";
humanreadable[17]="postfix_expression";
humanreadable[18]="postfix_expression";
humanreadable[19]="postfix_expression";
humanreadable[20]="postfix_expression";
humanreadable[21]="postfix_expression";
humanreadable[22]="postfix_expression";
humanreadable[23]="postfix_expression";
humanreadable[24]="postfix_expression";
humanreadable[25]="postfix_expression";

humanreadable[26]="argument_expression_list";
humanreadable[27]="argument_expression_list";

humanreadable[28]="unary_expression";
humanreadable[29]="unary_expression";
humanreadable[30]="unary_expression";
humanreadable[31]="unary_expression";
humanreadable[32]="unary_expression";
humanreadable[33]="unary_expression";
humanreadable[34]="unary_expression";

humanreadable[35]="unary_operator";
humanreadable[36]="unary_operator";
humanreadable[37]="unary_operator";
humanreadable[38]="unary_operator";
humanreadable[39]="unary_operator";
humanreadable[40]="unary_operator";

humanreadable[41]="cast_expression";
humanreadable[42]="cast_expression";

humanreadable[43]="multiplicative_expression";
humanreadable[44]="multiplicative_expression";
humanreadable[45]="multiplicative_expression";
humanreadable[46]="multiplicative_expression";

humanreadable[47]="additive_expression";
humanreadable[48]="additive_expression";
humanreadable[49]="additive_expression";

humanreadable[50]="shift_expression";
humanreadable[51]="shift_expression";
humanreadable[52]="shift_expression";

humanreadable[53]="relational_expression";
humanreadable[54]="relational_expression";
humanreadable[55]="relational_expression";
humanreadable[56]="relational_expression";
humanreadable[57]="relational_expression";

humanreadable[58]="equality_expression";
humanreadable[59]="equality_expression";
humanreadable[60]="equality_expression";

humanreadable[61]="and_expression";
humanreadable[62]="and_expression";


humanreadable[63]="exclusive_or_expression";
humanreadable[64]="exclusive_or_expression";

humanreadable[65]="inclusive_or_expression";
humanreadable[66]="inclusive_or_expression";

humanreadable[67]="logical_and_expression";
humanreadable[68]="logical_and_expression";

humanreadable[69]="logical_or_expression";
humanreadable[70]="logical_or_expression";

humanreadable[71]="conditional_expression";
humanreadable[72]="conditional_expression";

humanreadable[73]="assignment_expression";
humanreadable[74]="assignment_expression";

humanreadable[75]="assignment_operator";
humanreadable[76]="assignment_operator";
humanreadable[77]="assignment_operator";
humanreadable[78]="assignment_operator";
humanreadable[79]="assignment_operator";
humanreadable[80]="assignment_operator";
humanreadable[81]="assignment_operator";
humanreadable[82]="assignment_operator";
humanreadable[83]="assignment_operator";
humanreadable[84]="assignment_operator";
humanreadable[85]="assignment_operator";


humanreadable[86]="expression";
humanreadable[87]="expression";

humanreadable[88]="constant_expression";

humanreadable[89]="declaration";
humanreadable[90]="declaration";
humanreadable[91]="declaration";

humanreadable[92]="declaration_specifiers";
humanreadable[93]="declaration_specifiers";
humanreadable[94]="declaration_specifiers";
humanreadable[95]="declaration_specifiers";
humanreadable[96]="declaration_specifiers";
humanreadable[97]="declaration_specifiers";
humanreadable[98]="declaration_specifiers";
humanreadable[99]="declaration_specifiers";
humanreadable[100]="declaration_specifiers";
humanreadable[101]="declaration_specifiers";

humanreadable[102]="init_declarator_list";
humanreadable[103]="init_declarator_list";

humanreadable[104]="init_declarator";
humanreadable[105]="init_declarator";

humanreadable[106]="storage_class_specifier";
humanreadable[107]="storage_class_specifier";
humanreadable[108]="storage_class_specifier";
humanreadable[109]="storage_class_specifier";
humanreadable[110]="storage_class_specifier";
humanreadable[111]="storage_class_specifier";

humanreadable[112]="type_specifier";
humanreadable[113]="type_specifier";
humanreadable[114]="type_specifier";
humanreadable[115]="type_specifier";
humanreadable[116]="type_specifier";
humanreadable[117]="type_specifier";
humanreadable[118]="type_specifier";
humanreadable[119]="type_specifier";
humanreadable[120]="type_specifier";
humanreadable[121]="type_specifier";
humanreadable[122]="type_specifier";
humanreadable[123]="type_specifier";
humanreadable[124]="type_specifier";
humanreadable[125]="type_specifier";
humanreadable[126]="type_specifier";

humanreadable[127]="struct_or_union_specifier";
humanreadable[128]="struct_or_union_specifier";
humanreadable[129]="struct_or_union_specifier";

humanreadable[130]="struct_or_union";
humanreadable[131]="struct_or_union";

humanreadable[132]="struct_declaration_list";
humanreadable[133]="struct_declaration_list";

humanreadable[134]="struct_declaration";
humanreadable[135]="struct_declaration";
humanreadable[136]="struct_declaration";

humanreadable[137]="specifier_qualifier_list";
humanreadable[138]="specifier_qualifier_list";
humanreadable[139]="specifier_qualifier_list";
humanreadable[140]="specifier_qualifier_list";

humanreadable[141]="struct_declarator_list";
humanreadable[142]="struct_declarator_list";

humanreadable[143]="struct_declarator";
humanreadable[144]="struct_declarator";
humanreadable[145]="struct_declarator";

humanreadable[146]="enum_specifier";
humanreadable[147]="enum_specifier";
humanreadable[148]="enum_specifier";
humanreadable[149]="enum_specifier";
humanreadable[150]="enum_specifier";

humanreadable[151]="enumerator_list";
humanreadable[152]="enumerator_list";

humanreadable[153]="enumerator";
humanreadable[154]="enumerator";

humanreadable[155]="atomic_type_specifier";

humanreadable[156]="type_qualifier";
humanreadable[157]="type_qualifier";
humanreadable[158]="type_qualifier";
humanreadable[159]="type_qualifier";

humanreadable[160]="function_specifier";
humanreadable[161]="function_specifier";

humanreadable[162]="alignment_specifier";
humanreadable[163]="alignment_specifier";

humanreadable[164]="declarator";
humanreadable[165]="declarator";

humanreadable[166]="direct_declarator";
humanreadable[167]="direct_declarator";
humanreadable[168]="direct_declarator";
humanreadable[169]="direct_declarator";
humanreadable[170]="direct_declarator";
humanreadable[171]="direct_declarator";
humanreadable[172]="direct_declarator";
humanreadable[173]="direct_declarator";
humanreadable[174]="direct_declarator";
humanreadable[175]="direct_declarator";
humanreadable[176]="direct_declarator";
humanreadable[177]="direct_declarator";
humanreadable[178]="direct_declarator";
humanreadable[179]="direct_declarator";

humanreadable[180]="pointer";
humanreadable[181]="pointer";
humanreadable[182]="pointer";
humanreadable[183]="pointer";

humanreadable[184]="type_qualifier_list";
humanreadable[185]="type_qualifier_list";

humanreadable[186]="parameter_type_list";
humanreadable[187]="parameter_type_list";

humanreadable[188]="parameter_list";
humanreadable[189]="parameter_list";

humanreadable[190]="parameter_declaration";
humanreadable[191]="parameter_declaration";
humanreadable[192]="parameter_declaration";

humanreadable[193]="identifier_list";
humanreadable[194]="identifier_list";

humanreadable[195]="type_name";
humanreadable[196]="type_name";

humanreadable[197]="abstract_declarator";
humanreadable[198]="abstract_declarator";
humanreadable[199]="abstract_declarator";


humanreadable[200]="direct_abstract_declarator";
humanreadable[201]="direct_abstract_declarator";
humanreadable[202]="direct_abstract_declarator";
humanreadable[203]="direct_abstract_declarator";
humanreadable[204]="direct_abstract_declarator";
humanreadable[205]="direct_abstract_declarator";
humanreadable[206]="direct_abstract_declarator";
humanreadable[207]="direct_abstract_declarator";
humanreadable[208]="direct_abstract_declarator";
humanreadable[209]="direct_abstract_declarator";
humanreadable[210]="direct_abstract_declarator";
humanreadable[211]="direct_abstract_declarator";
humanreadable[212]="direct_abstract_declarator";
humanreadable[213]="direct_abstract_declarator";
humanreadable[214]="direct_abstract_declarator";
humanreadable[215]="direct_abstract_declarator";
humanreadable[216]="direct_abstract_declarator";
humanreadable[217]="direct_abstract_declarator";
humanreadable[218]="direct_abstract_declarator";
humanreadable[219]="direct_abstract_declarator";
humanreadable[220]="direct_abstract_declarator";

//humanreadable[57]="typedef_name";
humanreadable[221]="initializer";
humanreadable[222]="initializer";
humanreadable[223]="initializer";

humanreadable[224]="initializer_list";
humanreadable[225]="initializer_list";
humanreadable[226]="initializer_list";
humanreadable[227]="initializer_list";

humanreadable[228]="designation";

humanreadable[229]="designator_list";
humanreadable[230]="designator_list";

humanreadable[231]="designator";
humanreadable[232]="designator";

humanreadable[233]="static_assert_declaration";


humanreadable[234]="statement";
humanreadable[235]="statement";
humanreadable[236]="statement";
humanreadable[237]="statement";
humanreadable[238]="statement";
humanreadable[239]="statement";

humanreadable[240]="labeled_statement";
humanreadable[241]="labeled_statement";
humanreadable[242]="labeled_statement";

humanreadable[243]="compound_statement";
humanreadable[244]="compound_statement";

humanreadable[245]="block_item_list";
humanreadable[246]="block_item_list";

humanreadable[247]="block_item";
humanreadable[248]="block_item";

humanreadable[249]="expression_statement";
humanreadable[250]="expression_statement";

humanreadable[251]="selection_statement";
humanreadable[252]="selection_statement";
humanreadable[253]="selection_statement";

humanreadable[254]="iteration_statement";
humanreadable[255]="iteration_statement";
humanreadable[256]="iteration_statement";
humanreadable[257]="iteration_statement";
humanreadable[258]="iteration_statement";
humanreadable[259]="iteration_statement";
humanreadable[260]="iteration_statement";
humanreadable[261]="iteration_statement";
humanreadable[262]="iteration_statement";
humanreadable[263]="iteration_statement";
humanreadable[264]="iteration_statement";
humanreadable[265]="iteration_statement";
humanreadable[266]="iteration_statement";
humanreadable[267]="iteration_statement";

humanreadable[268]="jump_statement";
humanreadable[260]="jump_statement";
humanreadable[270]="jump_statement";
humanreadable[271]="jump_statement";
humanreadable[272]="jump_statement";

humanreadable[273]="translation_unit";
humanreadable[274]="translation_unit";

humanreadable[275]="external_declaration";
humanreadable[276]="external_declaration";

humanreadable[277]="function_definition";
humanreadable[278]="function_definition";

humanreadable[279]="declaration_list";
humanreadable[280]="declaration_list";

humanreadable[281]="file";
}

