#include "symtab.h"
#include "tac.h"

typeptr integer_type;
typeptr string_type;
typeptr char_type;
typeptr array_type;
typeptr function_type;
typeptr error_type;
typeptr void_type;
typeptr print_type;
typeptr getchar_type;
typeptr table_type;
typeptr universal_type;
typeptr char_array_type;
int semantic_error;

SymbolTable globals;
SymbolTable current;

void error(char*s,nodeptr t)
{
	if(t!=NULL)
	{
	while(t->nkids>0)
		t=t->kids[0];
	fprintf(stderr,"error: file: %s in line %d:%s\n",t->leaf->filename,t->leaf->lineno,s);
	}
	else
	fprintf(stderr,"Tree node pointer into error function is NULL!\n");
	semantic_error++;
}


void warn(char*s,nodeptr t)
{
	if(t!=NULL)
	{
	while(t->nkids>0)
		t=t->kids[0];
	fprintf(stderr,"warning: file: %s in line %d:%s\n",t->leaf->filename,t->leaf->lineno,s);
	}
	else
	fprintf(stderr,"Tree node pointer into warn function is NULL!\n");
}

void populatesymbols(nodeptr t)
{
	int i;
	char* s;
	if(t==NULL) return;

	switch(t->prodrule)
	{

		case function_definition:		//declaration_specifiers+declarator+declaration_list+compound_statement
		error("ct does not support pre-ANSI C standard",t);
		break;
		case function_definition+1:		//declaration_specifiers+declarator+compound_statement
		if(current->parent==NULL)//global
		{
			if(s=get_funcname(t->kids[1]))
			{
				boolSTE ste;
				typeptr typ=synthesize_type(t->kids[0]);//k->kids[0] is declaration_specifiers
				typeptr n=(typeptr)malloc(1*sizeof(struct typeinfo));
				n->basetype=5;//function_type
				n->u.f.parameters=create_param_list(peelparams(t->kids[1]));
				n->u.f.returntype=typ;
				ste=insert_st(current,s,n,0);
				if(ste.entry==NULL)
					{
						char* temp=(char*)malloc((strlen(s)+1+25)*sizeof(char));
						sprintf(temp,"redeclared function: '%s'",s);
						error(temp,t);
						free(temp);
						temp=NULL;
						break;
					}
				SymbolTable nst=new_st(13,ARG);//ARG means parameters scope
				//pushscope(nst);
				nst->parent=current;
				current=nst;
				givescope(peelparams(t->kids[1]),current);
				populateparams(peelparams(t->kids[1]));	//k->kids[1] is declarator
				nst=new_st(13,LOCAL);
				nst->parent=current;
				current=nst;
				givescope(t->kids[2],current);
				populatelocals(t->kids[2]);	//k->kids[2] is compound_statement
				//popscople();
				current=current->parent;	//pop to param scope
				current=current->parent;	//pop to global scope
			} else error("Can't find function name",t);
		}
		else
			error("function definition cannot be put in another function",t);
			break;
		case declaration:	//declaration_specifiers+init_declarator_list+SM
			if(t->nkids>0)//to skip SM case
			{
			typeptr typ=synthesize_type(t->kids[0]);
			populate_init_declarators(t->kids[1],typ);
			}
			break;
		case declaration+1:	//declaration_specifiers+SM
				error("the ct does not supported atomic,struct,union,enumerate,typedef!",t);
				break;
		case declaration+2:	//static_assert_declaration
				error("the ct does not supported static assert declaration!",t);
				break;
	}

	if(t->nkids>0)
		{
			for(i=0;i<t->nkids;i++)
				populatesymbols(t->kids[i]);
		}
}

typeptr synthesize_type(nodeptr t)
{
	int i;
	typeptr t1;
	if(t==NULL) return error_type;
	if(t->nkids>0)//if we are a non-terminal...
	{
		for(i=0;i<t->nkids;i++)
		{
			t1=synthesize_type(t->kids[i]);
			if(t1!=error_type)
			{
				return t1;
			}
		}
	}
	else //terminal
	{
		switch(t->leaf->category)
		{
			case INT:
				return integer_type;
			case VOID:
				return void_type;
			case CHAR:
				return char_type;
			case TABLE:
				return table_type;
			default:
				{
				char* temp=(char*)malloc((strlen(t->leaf->text)+1+28)*sizeof(char));
				sprintf(temp,"'%s' is not a supported type!",t->leaf->text);
				error(temp,t);
				free(temp);
				temp=NULL;
				return error_type;
				}
		}
	}
}



void populate_init_declarators(nodeptr t, typeptr typ)
{
	switch(t->prodrule)
	{
		case init_declarator_list:	//init_declarator
			populate_init_declarators(t->kids[0],typ);
			break;
		case init_declarator_list+1:	//init_declarator_list+CM+init_declarator_list
			if(t->nkids>0)
			{
			populate_init_declarators(t->kids[0],typ);
			populate_init_declarators(t->kids[2],typ);
			}
			break;
		case init_declarator:		//declarator
			populate_init_declarators(t->kids[0],typ);
			break;
		case init_declarator+1:		//declarator+ASN+initializer
			//error("initializer ignored!",t->kids[2]);
			populate_init_declarators(t->kids[0],typ);
			break;
		case declarator:		//pointer+direct_declarator
			error("pointers are not support in ct!",t->kids[0]);
			//populate_init_declarators(t->kids[1],typ);
			break;
		case declarator+1:		//direct_declarator
			populate_init_declarators(t->kids[0],typ);
			break;
		case direct_declarator:		//identifier
		{
			boolSTE ste;
			if(current->parent!=NULL) //check if it is local
			{
				if (isfunctiontype(typ)==1)
					{
						error("Function declaration cannot in another function definition",t);
						break;
					}
				else
					{
						ste=insert_st(current,t->kids[0]->leaf->text,typ,1);
						if(ste.correct==0)
							fprintf(stderr,"file: %s in line %d: identifier '%s' has already been declared!\n",t->kids[0]->leaf->filename,t->kids[0]->leaf->lineno,t->kids[0]->leaf->text);
						if(istabletype(typ)==1)
							{
							address size,new;
							size.region=IMM;
							size.offset=10;
							new.region=LOCAL;
							new.offset=0;
							new.text="new_st";
							address t1=newtemp(array_type,t->st);
							t->code=newinstr(PARAML,size,empty,empty);
							t->code=concatecode(t->code,newinstr(CALL,t1,new,size));
							ste.entry->tableaddress=t1;
							}
					}
			}
			else	//global
			{
				ste=insert_st(current,t->kids[0]->leaf->text,typ,1);
				if(ste.correct==0)	// identifier is in the symbol table
					{
						if (ste.entry==NULL)	//function prototype is in the symbol table
							break;
						else			//function definition or global variable is in the symbol table
							fprintf(stderr,"file: %s in line %d: identifier '%s' has already been declared!\n",t->kids[0]->leaf->filename,t->kids[0]->leaf->lineno,t->kids[0]->leaf->text);
					}
						if(istabletype(typ)==1)
							error("Does not support global table declaration",t);
			}
		break;
		}
		case direct_declarator+1:	//LP+declarator+RP
			if(t->nkids>0)//to skip LP or RP case
			populate_init_declarators(t->kids[1],typ);
			break;
		case direct_declarator+2:	//direct_declarator+LB+type_qualifier_list+assignment_expression+RB
			{
			char temp[]="ct does not support CONST,RESTRICT,VOLATILE,_ATOMIC type qualifiers!";
			error(temp,t);
			}
			break;
		case direct_declarator+3:	//direct_declarator+LB+assignment_expression+RB
			if(t->nkids>0)
			{
			typeptr n=(typeptr)malloc(1*sizeof(struct typeinfo));
			n->basetype=4;//integer array or char array array_type;
			n->u.a.size=get_subscript_value(t->kids[2]);
			n->u.a.elemtype=typ;
			populate_init_declarators(t->kids[0],n);
			}
			break;
		case direct_declarator+4:	//direct_declarator+LB+type_qualifier_list+RB
			{
			char* temp="";
			sprintf(temp,"ct does not support CONST,RESTRICT,VOLATILE,_ATOMIC type qualifiers!");
			error(temp,t);
			}
			break;
		case direct_declarator+5:	//direct_declarator+LB+RB  -> empty array
			if(t->nkids>0)
			{
			typeptr n=(typeptr)malloc(1*sizeof(struct typeinfo));
			n->basetype=4;//array_type;
			n->u.a.size=0;
			n->u.a.elemtype=typ;
			populate_init_declarators(t->kids[0],n);
			}
			break;
		case direct_declarator+6:	//direct_declarator+LB+STATIC+type_qualifier_list+assignment_expression+RB
			{
			char temp[]="ct does not support STATIC specifier and CONST,RESTRICT,VOLATILE,_ATOMIC type qualifiers!";
			error(temp,t);
			}
			break;
		case direct_declarator+7:	//direct_declarator+LB+STATIC+assignment_expression+RB
			{
			char temp[]="ct does not support STATIC specifier!";
			error(temp,t);
			}
			break;
		case direct_declarator+8:	//direct_declarator+LB+type_qualifier_list+STATIC+assignment_expression+RB
			{
			char temp[]="ct does not support CONST,RESTRICT,VOLATILE,_ATOMIC type qualifiers and STATIC specifier!";
			error(temp,t);
			}
			break;
		case direct_declarator+9:	//direct_declarator+LB+type_qualifier_list+MUL+RB
			{
			char temp[]="ct does not support CONST,RESTRICT,VOLATILE,_ATOMIC type qualifiers and pointer!";
			error(temp,t);
			}
			break;
		case direct_declarator+10:	//direct_declarator+LB+MUL+RB
			{
			char temp[]="ct does not support pointer!";
			error(temp,t);
			}
			break;
		case direct_declarator+11:	//direct_declarator+LP+parameter_type_list+RP
			if(t->nkids>0)
			{
			typeptr n=(typeptr)malloc(1*sizeof(struct typeinfo));
			n->basetype=5;//function_type;
			n->u.f.parameters=create_param_list(t->kids[2]);
			n->u.f.returntype=typ;
			populate_init_declarators(t->kids[0],n);			
			}
			break;
		case direct_declarator+12:	//direct_declarator+LP+identifier_list+RP
			if(t->nkids>0)
			{
			warn("parameter names (without types) in function declaration",t);
			typeptr n=(typeptr)malloc(1*sizeof(struct typeinfo));
			n->basetype=5;//function_type;
			n->u.f.parameters=create_param_list(t->kids[2]);
			n->u.f.returntype=typ;
			populate_init_declarators(t->kids[0],n);			
			}
			break;
		case direct_declarator+13:	//direct_declarator+LP+RP
			if(t->nkids>0)
			{
			typeptr n=(typeptr)malloc(1*sizeof(struct typeinfo));
			n->basetype=5;//function_type;
			n->u.f.parameters=NULL;
			n->u.f.returntype=typ;
			populate_init_declarators(t->kids[0],n);			
			}
			break;
		default:
			printf("There must be something wrong if the program goes here->populate_init_declarators()->default case\n");
			break;			
	}
}


paramlist create_param_list(nodeptr t)
{
	if(t==NULL) return NULL;	//if there is no paramlist in the function, then return NULL;
	switch(t->prodrule)
	{
		case parameter_type_list:	//parameter_list
			return create_param_list(t->kids[0]);
		case parameter_type_list+1:	//parameter_list+CM+ELIPSIS
			if(t->nkids>0)
			return create_param_list(t->kids[0]);
			else return NULL;		
		case parameter_list:		//parameter_declaration
			 return create_param_list(t->kids[0]);
		case parameter_list+1:		//parameter_list+CM+parameter_declaration
			//if(t->nkids>0)
			{
				paramlist n1,n2,p;
				n1=create_param_list(t->kids[0]);
				p=n1;
				if(p!=NULL)
				{
				while(p->next!=NULL)
					p=p->next;
				n2=create_param_list(t->kids[2]);
				p->next=n2;
				}
				return n1;
			}
		case parameter_declaration:	//declaration_specifier+declarator
			{	
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				//here need get name//
				typeptr tmp;
				tmp=synthesize_type(t->kids[0]);//here is just the base type
				if(t->kids[1]->kids[0]->prodrule==direct_declarator+5)//decompose declarator to see if it has identifier+LB+RB;
					{
						n->type=(typeptr)malloc(1*sizeof(struct typeinfo));
						n->type->basetype=4;
						n->type->u.a.elemtype=tmp;
					}
				else
					n->type=tmp;
				n->next=NULL;
				return n;
			}
		case parameter_declaration+1:	//declaration_specifier+abstract_declarator
			{	
				error("ct does not support abstract declarator!",t);
				/*
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				//here need get name//
				n->type=synthesize_type(t->kid[0]);
				n->next=NULL;
				return n;*/
				return NULL;
			}
		case parameter_declaration+2:	//declaration_specifier
			{	
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n->name=NULL;
				n->type=synthesize_type(t->kids[0]);
				n->next=NULL;
				return n;
			}
		case identifier_list:		//IDENTIFIER
			{
				warn("no type data will upgrade to int type",t);	
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n->name=t->leaf->text;
				n->type=integer_type;	//by default upgrade to int type if no type declared
				n->next=NULL;
				return n;
			}
		case identifier_list+1:		//identifier_list+CM+IDENTIFIER
			if(t->nkids>0)
			{	
				paramlist n1,n2,p;
				n1=create_param_list(t->kids[0]);
				p=n1;
				if(p!=NULL)
				{
				while(p->next!=NULL)
					p=p->next;
				n2=create_param_list(t->kids[2]);
				p->next=n2;
				}
				return n1;
			}

		default:
			printf("There must be something wrong if the program goes here->create_param_list()->default case\n");
			return NULL;			
	}
}

nodeptr peelparams(nodeptr t)	//peel several levels from declarator to parameter_type_list or identifier_list
{
		switch(t->prodrule)
		{
		case declarator:		//pointer direct_declarator
			error("ct does not support pointer!",t);
			return NULL;
		case declarator+1:		//direct_declarator
			return peelparams(t->kids[0]);//need to peel another skin
		case direct_declarator:		//identifier
			error("This is not a function,program should not be here",t);
			return NULL;
		case direct_declarator+1:	//LP+declarator+RP
			return peelparams(t->kids[1]);//need to peel another skin
		case direct_declarator+2:
		case direct_declarator+3:
		case direct_declarator+4:
		case direct_declarator+5:
		case direct_declarator+6:
		case direct_declarator+7:
		case direct_declarator+8:
		case direct_declarator+9:
		case direct_declarator+10:
			error("This is not a function,program should not be here",t);
			return NULL;
		case direct_declarator+11:	//direct_declarator+LP+parameter_type_list+RP
			t=t->kids[2];		//this structure is exactly we want, so we don't have peel another skin, just t=t->kids[2] to select parameter_type_list to next step
			return t;
		case direct_declarator+12:	//direct_declarator+LP+identifier_list+RP
			t=t->kids[2];
			return t;
		case direct_declarator+13:	//direct_declarator+LP+RP
			return NULL;	
		}

}
void populateparams(nodeptr t)
{
	if(t==NULL) return; //t could be NULL since peelparams() could return NULL 


		//here t should be peeled enough to the case that only parameter_type_list or identifier_list left to be processed.

			switch(t->prodrule)
			{
				case parameter_type_list:	//parameter_list
					populateparams(t->kids[0]);
					break;
				case parameter_type_list+1:	//parameter_list+CM+ELIPSIS
					populateparams(t->kids[0]);
					break;
				case parameter_list:		//parameter_declaration
					populateparams(t->kids[0]);
					break;
				case parameter_list+1:		//parameter_list+CM+parameter_declaration
					populateparams(t->kids[0]);
					populateparams(t->kids[2]);
					break;
				case parameter_declaration:	//declaration_specifiers+declartor
					{
					//typeptr n=(typeptr)malloc(1*sizeof(struct typeinfo));
					typeptr typ=synthesize_type(t->kids[0]);
					populate_init_declarators(t->kids[1],typ);
					}
					break;
				case parameter_declaration+1:	//declaration_specifiers+abstract_declartor
					error("ct does not support abstract declarator",t);
					break;
				case parameter_declaration+2:	//declaration_specifiers
					error("parameter name omitted",t);
					break;
			}

}

void givescope(nodeptr t,SymbolTable st)
{

	if (t==NULL) return;
	int i;
/*	switch(t->prodrule)
	{
		case identifier:
			if(t->nkids==0)
			t->st=st;
			break;
		case primary_expression:
			if(t->nkids==0)
			t->st=st;
			break;
	}*/

	t->st=st; //give every tree node of current symboltable scope as long as it is in the function definition's compound statement. Not only for identifiers
	for(i=0;i<t->nkids;i++)
		givescope(t->kids[i],st);
	
}
void populatelocals(nodeptr t)	//since no nested locals, so just no worry about create a new scope
{
	int i;
	switch(t->prodrule)
	{

		case block_item:		//declaration
			populatesymbols(t->kids[0]);
			break;
		default:
			for(i=0;i<t->nkids;i++)
				populatelocals(t->kids[i]);
			break;
	}
}

char* get_funcname(nodeptr t) //from declarator into this function
{
	int seenfunc=0;
	while(1)
	{
		switch(t->prodrule)
		{

			case declarator:		//pointer+direct_declarator
				error("ct does not support pointer!",t);
				//t=t->kids[1];
				//break;
				return NULL;
			case declarator+1:		//direct_declarator
				t=t->kids[0];
				break;
			case direct_declarator:		//identifier
				if(seenfunc) return t->kids[0]->leaf->text;
				else return NULL;
			case direct_declarator+1:	//LP+declarator+RP
				t=t->kids[1];
				break;
			case direct_declarator+2:
			case direct_declarator+3:
			case direct_declarator+4:
			case direct_declarator+5:
			case direct_declarator+6:
			case direct_declarator+7:
			case direct_declarator+8:
			case direct_declarator+9:
			case direct_declarator+10:
				error("arrays not allowed in function headers",t);
				return NULL;
			case direct_declarator+11:
			case direct_declarator+12:
			case direct_declarator+13:
				seenfunc=1;
				t=t->kids[0];
				break;
		}
	}
}
SymbolTableEntry check_id_all_scope(char* s,SymbolTable st)
{
	if(st==NULL)
		return NULL;
	SymbolTableEntry ste;
	SymbolTableEntry result;
	ste=lookup_st(st,s);

	if(ste==NULL)
		result=check_id_all_scope(s,st->parent);
	else
		return ste;

	return result;
}
void typechecking(nodeptr t)
{
	int i;
	char*s;
	if(t==NULL) return;

	for(i=0;i<t->nkids;i++)
		typechecking(t->kids[i]);

	switch(t->prodrule)
	{
		case constant:		//ICON
			t->type=integer_type;
			return;
		case constant+1:	//FCON
			error("ct does not support float type",t);
			return;
		case constant+2:	//ENUMERATION_CONSTANT
			error("ct does not support enum type",t);
			return;
		case constant+3:	//CCON
			t->type=char_type;
			return;
		case primary_expression:	//IDENTIFIER
			{
			SymbolTableEntry ste;
			if(t->st==NULL)//global symbol table
				t->st=globals;
			ste=check_id_all_scope(t->leaf->text,t->st);

			if(ste!=NULL)
				t->type=ste->type;
			else
				{
					t->type=error_type;
					char temp[1024];
					sprintf(temp,"'%s' undeclared (first use in this function)",t->leaf->text);
					error(temp,t);
				}



/*
			ste=lookup_st(t->st,t->leaf->text);
			if(ste==NULL)
			{
				if(t->st->parent!=NULL)
				{
					ste1=lookup_st(t->st->parent,t->leaf->text);
					if(ste1==NULL)
					{
					t->type=error_type;
					char temp[1024];
					sprintf(temp,"'%s' undeclared (first use in this function)",t->leaf->text);
					error(temp,t);
					//error("(Each undeclared identifier is reported only once for each function it appears in.)",t);
					} 
					else 
					t->type=ste1->type;
				}
				else
				{
					t->type=error_type;
					char temp[1024];
					sprintf(temp,"'%s' undeclared (first use in this function)",t->leaf->text);
					error(temp,t);
					error("(Each undeclared identifier is reported only once for each function it appears in.)",t);
				}
			}
			else
			{
				t->type=ste->type;
			}*/
			return;
			}
		case primary_expression+1:	//constant
			t->type=t->kids[0]->type;
			return;
		case primary_expression+2:	//STRING_LITERAL
			t->type=string_type;
			//t->type->u.a.size=strlen(t->leaf->text);
			return;
		case primary_expression+3:	//LP+expression+RP
			if(t->nkids>0)
			{
			t->type=t->kids[1]->type;
			}
			return;
		case primary_expression+4:	//generic_selection
			error("ct does not support generic_selection expression",t);
			return;
		case postfix_expression:	//primary_expression
			t->type=t->kids[0]->type;
			return;
		case postfix_expression+1:	//postfix_expression+LB+expression+RB
		
			if(t->nkids>0)
			{
			if(t->kids[0]->type->basetype==5)//the identifier is function
				{
					error("Function cannot be an array or pointer!",t);
					t->type=error_type;
				}
			
			if(isintegertype(t->kids[2]->type))
				{
				if(istabletype(t->kids[0]->type))
					t->type=string_type;
				else //array type
				t->type=t->kids[0]->type->u.a.elemtype;//if there is [], we need the elemtype
				}
			else if(isstringtype(t->kids[2]->type))
				{
				t->type=string_type;//the vaule of this expression (table) is string 
				}
			
			else	{
				error("expecting integer or string in the [ ]",t);
				t->type=error_type;
				}
			}
				return;
		case postfix_expression+2:	//postfix_expression+LP+argument_expression_list+RP
			if(t->nkids>0) //t->nkids=0 must be LP or RP
			{
			if(iserrortype(t->kids[0]->type)!=1)
			{
			if(isfunctiontype(t->kids[0]->type)!=1)
					error("expecting function here",t);
				else
					{
					checkfunctionparam(t->kids[0]->type->u.f.parameters,t->kids[2]);
					t->type=t->kids[0]->type->u.f.returntype;
					}
			}
			}
			return;
		case postfix_expression+3:	//postfix_expression+LP+RP
			if(t->nkids>0) //t->nkids=0 must be LP or RP
			{
				if(iserrortype(t->kids[0]->type)!=1)
				{
					if(isfunctiontype(t->kids[0]->type)!=1)
						error("expecting function here",t);
					else
						t->type=t->kids[0]->type->u.f.returntype;
				}
			}
			return;
		case postfix_expression+4:	//postfix_expression+DOT+identifier
			error("ct does not support dot operator",t);
			return;	
		case postfix_expression+5:	//postfix_expression+FOLLOW+identifier
			error("ct does not support pointer arrow operator",t);
			return;
		case postfix_expression+6:	//postfix_expression+INCOP	
			error("ct does not support post increment operator",t);
			return;
		case postfix_expression+7:	//postfix_expression+DECOP	
			error("ct does not support post decrement operator",t);
			return;
		case postfix_expression+8:	//LP+type_name+RP+LC+initializer_list+RC	
			error("ct does not support (type name){initializer_list} postfix expression",t);
			return;
		case postfix_expression+9:	//LP+type_name+RP+LC+initializer_list+CM+RC
			error("ct does not support (type name){initializer_list,} postfix expression",t);
			return;
		case argument_expression_list:	//assignment_expression
			t->type=t->kids[0]->type;
			return;
		case argument_expression_list+1: //argument_expression_list+CM+assignment_expression
			if(t->nkids>0)
				t->type=t->kids[2]->type;
			return;
		case unary_expression:		//postfix_expression
			t->type=t->kids[0]->type;
			return;
		case unary_expression+1:	//INCOP unary_expression		
			error("ct does not support pre increment operator",t);
			return;
		case unary_expression+2:	//DECOP unary_expression		
			error("ct does not support pre decrement operator",t);
			return;	
		case unary_expression+3:	//unary_operator+cast_expression
			error("ct does not support unary operator",t);
			return;
		case unary_expression+4:	//SIZEOF+unary_expression
			error("ct does not support sizeof operator",t);
			return;
		case unary_expression+5:	//SIZEOF+LP+type_name+RP
			error("ct does not support sizeof operator",t);
			return;
		case unary_expression+6:	//_ALIGNOF+LP+type_name+RP
			error("ct does not support _ALIGNOF opereator",t);
			return;			
	
		//case unary_operator: omit all here

		case cast_expression:		//unary_expression
			t->type=t->kids[0]->type;
			return;
		case cast_expression+1:		//LP type_name RP cast_expression
			error("ct does not support type cast opereation",t);
			return;
		case multiplicative_expression:	//cast_expression
			t->type=t->kids[0]->type;
			return;
		case multiplicative_expression+1:	//multiplicative_expression+MUL+cast_expression
			if(t->nkids>0)
			{
			t->type=compare_types(t->kids[0]->type,t->kids[2]->type);
			if(iserrortype(t->type)==1)
				error("type conflict",t);
			}
			return;
		case multiplicative_expression+2:	//multiplicative_expression+DIV+cast_expression
			if(t->nkids>0)
			{
			t->type=compare_types(t->kids[0]->type,t->kids[2]->type);
			if(iserrortype(t->type)==1)
				error("type conflict",t);
			}
			return;
		case multiplicative_expression+3:	//multiplicative_expression+MOD+cast_expression
			error("ct does not support MOD opereator",t);
			return;
		case additive_expression:		//multiplicative_expression
			t->type=t->kids[0]->type;
			return;
		case additive_expression+1:		//additive_expression+PLUS+multiplicative_expression
			if(t->nkids>0)
			{
			t->type=compare_types(t->kids[0]->type,t->kids[2]->type);
			if(iserrortype(t->type)==1)
				error("type conflict",t);
			}
			return;
		case additive_expression+2:		//additive_expression+MINUS+multiplicative_expression
			if(t->nkids>0)
			{
			t->type=compare_types(t->kids[0]->type,t->kids[2]->type);
			if(iserrortype(t->type)==1)
				error("type conflict",t);
			}
			return;
		case shift_expression:			//additive_expression
			t->type=t->kids[0]->type;
			return;
		case shift_expression+1:		//shift_expression+SHL+additive_expression
			error("ct does not support bitwise left shift opereator",t);
			return;
		case shift_expression+2:		//shift_expression+SHR+additive_expression
			error("ct does not support bitwise right shift opereator",t);
			return;
		case relational_expression:		//shift_expression
			t->type=t->kids[0]->type;
			return;
		case relational_expression+1:		//relational_expression+LT+shift_expression
			if(t->nkids>0)
			{
			t->type=compare_types(t->kids[0]->type,t->kids[2]->type);
			if(iserrortype(t->type)==1)
				error("type conflict",t);
			}
			return;
		case relational_expression+2:		//relational_expression+GT+shift_expression
			if(t->nkids>0)
			{
			t->type=compare_types(t->kids[0]->type,t->kids[2]->type);
			if(iserrortype(t->type)==1)
				error("type conflict",t);
			}
			return;
		case relational_expression+3:		//relational_expression+LE+shift_expression
			if(t->nkids>0)
			{
			t->type=compare_types(t->kids[0]->type,t->kids[2]->type);
			if(iserrortype(t->type)==1)
				error("type conflict",t);
			}
			return;
		case relational_expression+4:		//relational_expression+GE+shift_expression
			if(t->nkids>0)
			{
			t->type=compare_types(t->kids[0]->type,t->kids[2]->type);
			if(iserrortype(t->type)==1)
				error("type conflict",t);
			}
			return;
		case equality_expression:		//relational_expression
			t->type=t->kids[0]->type;
			return;
		case equality_expression+1:		//equality_expression+EQ+relational_expression
			if(t->nkids>0)
			{
			t->type=compare_types(t->kids[0]->type,t->kids[2]->type);
			if(iserrortype(t->type)==1)
				error("type conflict",t);
			}
			return;
		case equality_expression+2:		//equality_expression+NE+relational_expression
			if(t->nkids>0)
			{
			t->type=compare_types(t->kids[0]->type,t->kids[2]->type);
			if(iserrortype(t->type)==1)
				error("type conflict",t);
			}
			return;
		case and_expression:			//equality_expression
			t->type=t->kids[0]->type;
			return;
		case and_expression+1:			//and_expression+AND+equality_expression			
			error("ct does not support bitwise and opereator",t);
			return;
		case exclusive_or_expression:		//and_expression
			t->type=t->kids[0]->type;
			return;
		case exclusive_or_expression+1:		//exclusive_or_expression+ER+and_expression
			error("ct does not support bitwise exclusive or opereator",t);
			return;
		case inclusive_or_expression:		//exclusive_or_expression
			t->type=t->kids[0]->type;
			return;
		case inclusive_or_expression+1:		//inclusive_or_expression+OR+exclusive_or_expression
			error("ct does not support bitwise inclusive or opereator",t);
			return;
		case logical_and_expression:		//inclusive_or_expression
			t->type=t->kids[0]->type;
			return;
		case logical_and_expression+1:		//logical_and_expression+ANDAND+inclusive_or_expression
			if(t->nkids>0)	
			{
			t->type=compare_types(t->kids[0]->type,t->kids[2]->type);
			if(iserrortype(t->type)==1)
				error("type conflict",t);
			}
			return;
		case logical_or_expression:		//logical_and_expression
			t->type=t->kids[0]->type;
			return;
		case logical_or_expression+1:		//logical_or_expression+OROR+logical_and_expression
			if(t->nkids>0)
			{
			t->type=compare_types(t->kids[0]->type,t->kids[2]->type);
			if(iserrortype(t->type)==1)
				error("type conflict",t);
			}
			return;
		case conditional_expression:		//logical_or_expression
			t->type=t->kids[0]->type;
			return;
		case conditional_expression+1:		//logical_or_expression+QUEST+expression+COLON+conditional_expression
			error("ct does not support conditional ? and : opereator",t);
			return;
		case assignment_expression:		//conditional_expression
			t->type=t->kids[0]->type;
			return;
		case assignment_expression+1:		//assignment_expression+assignment_ooperator+conditional_expression
			if(t->nkids>0)
			{
			t->type=compare_types(t->kids[0]->type,t->kids[2]->type);
			if(iserrortype(t->type)==1)
				error("type conflict",t);
			}
			return;
		case expression:			//assignment_expression
			t->type=t->kids[0]->type;
			return;
		case expression+1:			//assignment_expression+CM+assignment_expression
			t->type=t->kids[2]->type;
			return;
		case constant_expression:		//conditional_expression
			t->type=t->kids[0]->type;
			return;
		default:
			return;			
	}
}


paramlist create_argument_list(nodeptr t)
{
	switch(t->prodrule)
	{
		case argument_expression_list:	//assignment_expression
			return create_argument_list(t->kids[0]);
		case argument_expression_list+1:	//argument_expression_list+CM+assignment_expression
			{
				paramlist n1,n2,p;
				n1=create_argument_list(t->kids[0]);
				p=n1;
				if(p!=NULL)
				{
				while(p->next!=NULL)
					p=p->next;
				n2=create_argument_list(t->kids[2]);
				p->next=n2;
				}
			
				return n1;
			}	
		case assignment_expression:		//conditional_expression
			 return create_argument_list(t->kids[0]);
		case assignment_expression+1:		//unary_expression+assignment_operator+assignment_expression
				{
				paramlist n1=(paramlist)malloc(1*sizeof(struct param));
				paramlist n2=(paramlist)malloc(1*sizeof(struct param));
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n1=create_argument_list(t->kids[0]);
				n2=create_argument_list(t->kids[2]);
				n->type=compare_types(n1->type,n2->type);
				if(iserrortype(n->type)==1)
					error("type conflict",t);
				n->next=NULL;
				return n;
				}
		case conditional_expression:		//logical_or_expression
			return create_argument_list(t->kids[0]);
		case conditional_expression+1:		//logical_or_expression+QUEST+expresson+COLON+conditional_expression
			error("the argument does not support ?+:expresion",t);
			return NULL;			
		case logical_or_expression:		//logical_and_expression
			return create_argument_list(t->kids[0]);
		case logical_or_expression+1:		//logical_or_expression+OROR+logical_and_expression
				{
				paramlist n1=(paramlist)malloc(1*sizeof(struct param));
				paramlist n2=(paramlist)malloc(1*sizeof(struct param));
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n1=create_argument_list(t->kids[0]);
				n2=create_argument_list(t->kids[2]);
				n->type=compare_types(n1->type,n2->type);
				if(iserrortype(n->type)==1)
					error("type conflict",t);
				n->next=NULL;
				return n;
				}			
		case logical_and_expression:		//inclusive_or_expression
			return create_argument_list(t->kids[0]);
		case logical_and_expression+1:		//logical_and_expression+ANDAND+inclusive_or_expression
				{
				paramlist n1=(paramlist)malloc(1*sizeof(struct param));
				paramlist n2=(paramlist)malloc(1*sizeof(struct param));
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n1=create_argument_list(t->kids[0]);
				n2=create_argument_list(t->kids[2]);
				n->type=compare_types(n1->type,n2->type);
				if(iserrortype(n->type)==1)
					error("type conflict",t);
				n->next=NULL;
				return n;
				}
		case inclusive_or_expression:		//exclusive_or_expression
			return create_argument_list(t->kids[0]);
		case inclusive_or_expression+1:		//inclusive_or_expression+OR+exclusive_or_expression
			error("the argument does not support bitwise OR operator",t);
			return NULL;
		case exclusive_or_expression:		//and_expression
			return create_argument_list(t->kids[0]);
		case exclusive_or_expression+1:		//exclusive_or_expression+ER+and_expression
			error("the argument does not support bitwise XOR operator",t);
			return NULL;			
		case and_expression:			//equality_expression
			return create_argument_list(t->kids[0]);
		case and_expression+1:			//and_expression+AND+equality_expression
			error("the argument does not support bitwise AND operator",t);
			return NULL;			
		case equality_expression:		//relational expression
			return create_argument_list(t->kids[0]);
		case equality_expression+1:		//equality_expression+EQ+relational_expression
				{
				paramlist n1=(paramlist)malloc(1*sizeof(struct param));
				paramlist n2=(paramlist)malloc(1*sizeof(struct param));
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n1=create_argument_list(t->kids[0]);
				n2=create_argument_list(t->kids[2]);
				n->type=compare_types(n1->type,n2->type);
				if(iserrortype(n->type)==1)
					error("type conflict",t);
				n->next=NULL;
				return n;
				}			
		case equality_expression+2:		//equality_expression+NE+relational_expression
				{
				paramlist n1=(paramlist)malloc(1*sizeof(struct param));
				paramlist n2=(paramlist)malloc(1*sizeof(struct param));
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n1=create_argument_list(t->kids[0]);
				n2=create_argument_list(t->kids[2]);
				n->type=compare_types(n1->type,n2->type);
				if(iserrortype(n->type)==1)
					error("type conflict",t);
				n->next=NULL;
				return n;
				}
		case relational_expression:		//shift_expression
			return create_argument_list(t->kids[0]);
		case relational_expression+1:		//relational_expression+LT+shift_expression
				{
				paramlist n1=(paramlist)malloc(1*sizeof(struct param));
				paramlist n2=(paramlist)malloc(1*sizeof(struct param));
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n1=create_argument_list(t->kids[0]);
				n2=create_argument_list(t->kids[2]);
				n->type=compare_types(n1->type,n2->type);
				if(iserrortype(n->type)==1)
					error("type conflict",t);
				n->next=NULL;
				return n;
				}			
		case relational_expression+2:		//relational_expression+GT+shift_expression
				{
				paramlist n1=(paramlist)malloc(1*sizeof(struct param));
				paramlist n2=(paramlist)malloc(1*sizeof(struct param));
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n1=create_argument_list(t->kids[0]);
				n2=create_argument_list(t->kids[2]);
				n->type=compare_types(n1->type,n2->type);
				if(iserrortype(n->type)==1)
					error("type conflict",t);
				n->next=NULL;
				return n;
				}
		case relational_expression+3:		//relational_expression+LE+shift_expression
				{
				paramlist n1=(paramlist)malloc(1*sizeof(struct param));
				paramlist n2=(paramlist)malloc(1*sizeof(struct param));
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n1=create_argument_list(t->kids[0]);
				n2=create_argument_list(t->kids[2]);
				n->type=compare_types(n1->type,n2->type);
				if(iserrortype(n->type)==1)
					error("type conflict",t);
				n->next=NULL;
				return n;
				}
		case relational_expression+4:		//relational_expression+GE+shift_expression
				{
				paramlist n1=(paramlist)malloc(1*sizeof(struct param));
				paramlist n2=(paramlist)malloc(1*sizeof(struct param));
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n1=create_argument_list(t->kids[0]);
				n2=create_argument_list(t->kids[2]);
				n->type=compare_types(n1->type,n2->type);
				if(iserrortype(n->type)==1)
					error("type conflict",t);
				n->next=NULL;
				return n;
				}
		case shift_expression:			//additive_expression
			return create_argument_list(t->kids[0]);
		case shift_expression+1:		//shift_expression+SHL+additive_expression
			error("the argument does not support bitwise << operator",t);
			return NULL;			
		case shift_expression+2:		//shift_expression+SHR+additive_expression
			error("the argument does not support bitwise >> operator",t);
			return NULL;
		case additive_expression:		//multiplicative_expression
			return create_argument_list(t->kids[0]);
		case additive_expression+1:
				{
				paramlist n1=(paramlist)malloc(1*sizeof(struct param));
				paramlist n2=(paramlist)malloc(1*sizeof(struct param));
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n1=create_argument_list(t->kids[0]);
				n2=create_argument_list(t->kids[2]);
				n->type=compare_types(n1->type,n2->type);
				if(iserrortype(n->type)==1)
					error("type conflict",t);
				n->next=NULL;
				return n;
				}			
		case additive_expression+2:		//additive_expression+MINUS+multiplicative_expression
				{
				paramlist n1=(paramlist)malloc(1*sizeof(struct param));
				paramlist n2=(paramlist)malloc(1*sizeof(struct param));
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n1=create_argument_list(t->kids[0]);
				n2=create_argument_list(t->kids[2]);
				n->type=compare_types(n1->type,n2->type);
				if(iserrortype(n->type)==1)
					error("type conflict",t);
				n->next=NULL;
				return n;
				}			
		case multiplicative_expression:		//cast_expression
			return create_argument_list(t->kids[0]);
		case multiplicative_expression+1:
				{
				paramlist n1=(paramlist)malloc(1*sizeof(struct param));
				paramlist n2=(paramlist)malloc(1*sizeof(struct param));
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n1=create_argument_list(t->kids[0]);
				n2=create_argument_list(t->kids[2]);
				n->type=compare_types(n1->type,n2->type);
				if(iserrortype(n->type)==1)
					error("type conflict",t);
				n->next=NULL;
				return n;
				}			
		case multiplicative_expression+2:
				{
				paramlist n1=(paramlist)malloc(1*sizeof(struct param));
				paramlist n2=(paramlist)malloc(1*sizeof(struct param));
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n1=create_argument_list(t->kids[0]);
				n2=create_argument_list(t->kids[2]);
				n->type=compare_types(n1->type,n2->type);
				if(iserrortype(n->type)==1)
					error("type conflict",t);
				n->next=NULL;
				return n;
				}
		case multiplicative_expression+3:
			error("the argument does not support %% operator",t);
			return NULL;
		case cast_expression:			//unary_expression
			return create_argument_list(t->kids[0]);
		case cast_expression+1:
			error("the argument does not support cast type operation",t);
			return NULL;
		case unary_expression:			//postfix_expression
			return create_argument_list(t->kids[0]);
		case unary_expression+1:
			error("the argument does not support increment operation",t);
			return NULL;			
		case unary_expression+2:
			error("the argument does not support decrement operation",t);
			return NULL;
		case unary_expression+3:
			error("the argument does not support unary operator operation",t);
			return NULL;
		case unary_expression+4:
			error("the argument does not support sizeof() operation",t);
			return NULL;
		case unary_expression+5:
			error("the argument does not support sizeof() operation",t);
			return NULL;
		case unary_expression+6:
			error("the argument does not support _alignof() operation",t);
			return NULL;
		case postfix_expression:		//primary_expression
			return create_argument_list(t->kids[0]);
		case postfix_expression+1:
			return create_argument_list(t->kids[0]);
		case postfix_expression+2:
			return create_argument_list(t->kids[0]);
		case postfix_expression+3:
			return create_argument_list(t->kids[0]);
		case postfix_expression+4:
			error("the argument does not support dot operation",t);
			return NULL;
		case postfix_expression+5:
			error("the argument does not support -> operation",t);
			return NULL;
		case postfix_expression+6:
			error("the argument does not support post increment operation",t);
			return NULL;
		case postfix_expression+7:
			error("the argument does not support post decrement operation",t);
			return NULL;
		case postfix_expression+8:
			error("the argument does not support type cast operation",t);
			return NULL;
		case postfix_expression+9:
			error("the argument does not support type cast operation",t);
			return NULL;
		case primary_expression:	//IDENTIFIER
		{
			SymbolTableEntry ste;//,ste1;
			if(t->st==NULL)
				t->st=globals;

			ste=check_id_all_scope(t->leaf->text,t->st);
			paramlist n=(paramlist)malloc(1*sizeof(struct param));
			if(ste!=NULL)
				n->type=ste->type;
			else
				{
					n->type=error_type;
					char temp[1024];
					sprintf(temp,"'%s' undeclared (first use in this function)",t->leaf->text);
					error(temp,t);
				}

/*			ste=lookup_st(t->st,t->leaf->text);
			paramlist n=(paramlist)malloc(1*sizeof(struct param));
			if(ste!=NULL)
				n->type=ste->type;
			else
			{
				if(t->st->parent!=NULL)
				{
					ste1=lookup_st(t->st->parent,t->leaf->text);
					if(ste1==NULL)
					{
					n->type=error_type;
					char temp[1024];
					sprintf(temp,"'%s' undeclared (first use in this function)",t->leaf->text);
					error(temp,t);
					error("(Each undeclared identifier is reported only once for each function it appears in.)",t);
					}
					else
					n->type=ste1->type;
				}
				else
				{
					n->type=error_type;
					char temp[1024];
					sprintf(temp,"'%s' undeclared (first use in this function)",t->leaf->text);
					error(temp,t);
					error("(Each undeclared identifier is reported only once for each function it appears in.)",t);
				}
			}
*/			
			n->next=NULL;
			return n;
		}
		case primary_expression+1:		//constant
			return create_argument_list(t->kids[0]);
		case primary_expression+2:		//STRING_LITERAL
				{
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n->type=string_type;
				n->next=NULL;
				return n;
				}
		case primary_expression+3:		//LP+expression+RP
			return create_argument_list(t->kids[1]);
		case primary_expression+4:		
			error("the argument does not support generic_selection",t);
			return NULL;
		case constant:				//ICON
				{
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n->type=integer_type;
				n->next=NULL;
				return n;
				}			
		case constant+1:				//FCON
			error("the argument does not support float type",t);
			return NULL;
		case constant+2:				//ENUMERATION
			error("the argument does not support enum type",t);
			return NULL;
		case constant+3:				//CCON
				{
				paramlist n=(paramlist)malloc(1*sizeof(struct param));
				n->type=char_type;
				n->next=NULL;
				return n;
				}
		case expression:	//assignment_expression
			return create_argument_list(t->kids[0]);
		case expression+1:	//expression+CM+assignment_expression
				{
				paramlist n1,n2,p;
				n1=create_argument_list(t->kids[0]);
				p=n1;
				if(p!=NULL)
				{
				while(p->next!=NULL)
					p=p->next;
				n2=create_argument_list(t->kids[2]);
				p->next=n2;
				}
				return n1; 
				}
		case constant_expression:		//conditional_expression
			return create_argument_list(t->kids[0]);
		case assignment_operator:
		case assignment_operator+1:
		case assignment_operator+2:
		case assignment_operator+3:
		case assignment_operator+4:
		case assignment_operator+5:
		case assignment_operator+6:
		case assignment_operator+7:
		case assignment_operator+8:
		case assignment_operator+9:
		case assignment_operator+10:
			error("the argument does not support operator only",t);
			return NULL;
		default:
			printf("There must be something wrong if the program goes here->create_argument_list()->default case\n");
			return NULL;			
	}
}
void checkfunctionparam(paramlist l,nodeptr t)
{
	int n1=0,n2=0;
	paramlist c,p;
	c=create_argument_list(t);
	p=l;
	while(p!=NULL)
		{n1++;
		p=p->next;}
	p=c;
	while(p!=NULL)
		{n2++;p=p->next;}
	if(n1!=n2)
		error("The argument number is not as same as it is declared",t);
	else
		{
			int i;
			for(i=0;i<n1;i++)
				{
				if(compare_list_type(l,c,i)==-1)
				printf("The %dth argument type is not as same as it is declared!\n",i+1);
				}	
		}
	
}

int compare_list_type(paramlist l1,paramlist l2,int n)
{
	int i;
	paramlist p1,p2;
	p1=l1;
	p2=l2;
	for(i=0;i<n;i++)
		{
		p1=p1->next;p2=p2->next;
		}
	if(iserrortype(compare_types(p1->type,p2->type)))
		return -1;
	else {
		//p1->type=p2->type; //p1 is paramlist (prototype) maybe printf first param ->universal_type, it should not be replaced by the real type when calling->becase if call printf the second time, the 2rd argument could be another type not same as the first call. so it better to keep uniersal type. it does not matter other things, only in typechecking.
		return 1;
	     }
	
}

typeptr compare_types(typeptr t1,typeptr t2)
{
	if(t1!=NULL&&t2!=NULL)
	{
		if(t1->basetype==6||t2->basetype==6)
			{
				if(t1->basetype==6)	//return non-universal_type
					return t2;
				else
					return t1;
			}
		else if(t1->basetype!=t2->basetype)
			{
				if((t1->basetype==2 && t2->basetype==1)||(t1->basetype==1 && t2->basetype==2))	//upgrade char to int
					return ((t1->basetype==1)?t1:t2);
				else
				return error_type;
			}
		else
			{
			if(isarraytype(t1))
				{
				if(iserrortype(compare_types(t1->u.a.elemtype,t2->u.a.elemtype)))
					return error_type;
				else return t1;
				}
			else if(isfunctiontype(t1))
				{
				if(iserrortype(compare_types(t1->u.f.returntype,t2->u.f.returntype)))
					return error_type;
				else return t1;
				}
			return t1;
			}
	}
	else return error_type;
}


int get_subscript_value(nodeptr t)
{
	switch(t->prodrule)
	{
	
		case assignment_expression:		//conditional_expression
			 return get_subscript_value(t->kids[0]);
		case assignment_expression+1:		//unary_expression+assignment_operator+assignment_expression
			 return get_subscript_value(t->kids[2]);
		case conditional_expression:		//logical_or_expression
			return get_subscript_value(t->kids[0]);
		case conditional_expression+1:		//logical_or_expression+QUEST+expresson+COLON+conditional_expression
			error("ct does not support ?+:expresion",t);
			return 0;			
		case logical_or_expression:		//logical_and_expression
			return get_subscript_value(t->kids[0]);
		case logical_or_expression+1:		//logical_or_expression+OROR+logical_and_expression
			error("Need runtime code generation to get the expression value",t);
			return 0;			
		case logical_and_expression:		//inclusive_or_expression
			return get_subscript_value(t->kids[0]);
		case logical_and_expression+1:		//logical_and_expression+ANDAND+inclusive_or_expression
			error("Need runtime code generation to get the expression value",t);
			return 0;
		case inclusive_or_expression:		//exclusive_or_expression
			return get_subscript_value(t->kids[0]);
		case inclusive_or_expression+1:		//inclusive_or_expression+OR+exclusive_or_expression
			error("the argument does not support bitwise OR operator",t);
			return 0;
		case exclusive_or_expression:		//and_expression
			return get_subscript_value(t->kids[0]);
		case exclusive_or_expression+1:		//exclusive_or_expression+ER+and_expression
			error("the argument does not support bitwise XOR operator",t);
			return 0;			
		case and_expression:			//equality_expression
			return get_subscript_value(t->kids[0]);
		case and_expression+1:			//and_expression+AND+equality_expression
			error("the argument does not support bitwise AND operator",t);
			return 0;			
		case equality_expression:		//relational expression
			return get_subscript_value(t->kids[0]);
		case equality_expression+1:		//equality_expression+EQ+relational_expression
			error("Need runtime code generation to get the expression value",t);
			return 0;			
		case equality_expression+2:		//equality_expression+NE+relational_expression
			error("Need runtime code generation to get the expression value",t);
			return 0;
		case relational_expression:		//shift_expression
			return get_subscript_value(t->kids[0]);
		case relational_expression+1:		//relational_expression+LT+shift_expression
			error("Need runtime code generation to get the expression value",t);
			return 0;			
		case relational_expression+2:		//relational_expression+GT+shift_expression
			error("Need runtime code generation to get the expression value",t);
			return 0;
		case relational_expression+3:		//relational_expression+LE+shift_expression
			error("Need runtime code generation to get the expression value",t);
			return 0;
		case relational_expression+4:		//relational_expression+GE+shift_expression
			error("Need runtime code generation to get the expression value",t);
			return 0;
		case shift_expression:			//additive_expression
			return get_subscript_value(t->kids[0]);
		case shift_expression+1:		//shift_expression+SHL+additive_expression
			error("the argument does not support bitwise << operator",t);
			return 0;			
		case shift_expression+2:		//shift_expression+SHR+additive_expression
			error("the argument does not support bitwise >> operator",t);
			return 0;
		case additive_expression:		//multiplicative_expression
			return get_subscript_value(t->kids[0]);
		case additive_expression+1:
			error("Need runtime code generation to get the expression value",t);
			return 0;			
		case additive_expression+2:
			error("Need runtime code generation to get the expression value",t);
			return 0;
		case multiplicative_expression:		//cast_expression
			return get_subscript_value(t->kids[0]);
		case multiplicative_expression+1:
			error("Need runtime code generation to get the expression value",t);
			return 0;			
		case multiplicative_expression+2:
			error("Need runtime code generation to get the expression value",t);
			return 0;
		case multiplicative_expression+3:
			error("ct does not support %% operator",t);
			return 0;
		case cast_expression:			//unary_expression
			return get_subscript_value(t->kids[0]);
		case cast_expression+1:
			error("ct does not support cast type operation",t);
			return 0;
		case unary_expression:			//postfix_expression
			return get_subscript_value(t->kids[0]);
		case unary_expression+1:
			error("ct does not support increment operation",t);
			return 0;			
		case unary_expression+2:
			error("ct does not support decrement operation",t);
			return 0;
		case unary_expression+3:
			error("ct does not support unary operator operation",t);
			return 0;
		case unary_expression+4:
			error("ct does not support sizeof() operation",t);
			return 0;
		case unary_expression+5:
			error("ct does not support sizeof() operation",t);
			return 0;
		case unary_expression+6:
			error("ct does not support _alignof() operation",t);
			return 0;
		case postfix_expression:		//primary_expression
			return get_subscript_value(t->kids[0]);
		case postfix_expression+1:
			error("The array declaration subscript should be integer constant",t);
			return 0;
		case postfix_expression+2:
			error("The array declaration subscript should be integer constant",t);
			return 0;
		case postfix_expression+3:
			error("The array declaration subscript should be integer constant",t);
			return 0;
		case postfix_expression+4:
			error("ct does not support dot operation",t);
			return 0;
		case postfix_expression+5:
			error("ct does not support -> operation",t);
			return 0;
		case postfix_expression+6:
			error("ct does not support post increment operation",t);
			return 0;
		case postfix_expression+7:
			error("ct not support post decrement operation",t);
			return 0;
		case postfix_expression+8:
			error("ct does not support type cast operation",t);
			return 0;
		case postfix_expression+9:
			error("ct does not support type cast operation",t);
			return 0;
		case primary_expression:	//IDENTIFIER
			error("The array declaration subscript should be integer constant",t);
			return 0;
		case primary_expression+1:		//constant
			return get_subscript_value(t->kids[0]);
		case primary_expression+2:		//STRING_LITERAL
			error("The array declaration subscript should be integer constant",t);
			return 0;
		case primary_expression+3:		//LP+expression+RP
			return get_subscript_value(t->kids[1]);
		case primary_expression+4:		
			error("ct does not support generic_selection",t);
			return 0;
		case constant:				//ICON
			return t->leaf->ival;			
		case constant+1:				//FCON
			error("ct does not support float type",t);
			return 0;
		case constant+2:				//ENUMERATION
			error("ct does not support enum type",t);
			return 0;
		case constant+3:				//CCON
			error("The array declaration subscript should be integer constant",t);
			return 0;

		default:
			printf("There must be something wrong if the program goes here->create_argument_list()->default case\n");
			return 0;			
	}
}

void initial_types()
{
empty.region=0;
empty.offset=0;
empty.text=NULL;

globals=new_st(13,GLOBAL);

integer_type=(typeptr)malloc(1*sizeof(struct typeinfo));
integer_type->basetype=1;
integer_type->u.a.elemtype=NULL;
integer_type->u.f.st=NULL;
integer_type->u.f.returntype=NULL;
integer_type->u.f.parameters=NULL;

char_type=(typeptr)malloc(1*sizeof(struct typeinfo));
char_type->basetype=2;
char_type->u.a.elemtype=NULL;
char_type->u.f.st=NULL;
char_type->u.f.returntype=NULL;
char_type->u.f.parameters=NULL;

string_type=(typeptr)malloc(1*sizeof(struct typeinfo));
string_type->basetype=4;
string_type->u.f.st=NULL;
string_type->u.f.returntype=NULL;
string_type->u.f.parameters=NULL;
string_type->u.a.elemtype=char_type;


array_type=(typeptr)malloc(1*sizeof(struct typeinfo));
array_type->basetype=4;
array_type->u.a.elemtype=integer_type;
array_type->u.f.st=NULL;
array_type->u.f.returntype=NULL;
array_type->u.f.parameters=NULL;

table_type=(typeptr)malloc(1*sizeof(struct typeinfo));
table_type->basetype=3;
table_type->u.f.st=NULL;
table_type->u.f.returntype=NULL;
table_type->u.f.parameters=NULL;
table_type->u.a.elemtype=NULL;

function_type=(typeptr)malloc(1*sizeof(struct typeinfo));
function_type->basetype=5;
function_type->u.a.elemtype=NULL;
function_type->u.f.st=NULL;
function_type->u.f.returntype=NULL;
function_type->u.f.parameters=NULL;

error_type=(typeptr)malloc(1*sizeof(struct typeinfo));
error_type->basetype=0;
error_type->u.a.elemtype=NULL;
error_type->u.f.st=NULL;
error_type->u.f.returntype=NULL;
error_type->u.f.parameters=NULL;

void_type=(typeptr)malloc(1*sizeof(struct typeinfo));
void_type->basetype=-1;
void_type->u.a.elemtype=NULL;
void_type->u.f.st=NULL;
void_type->u.f.returntype=NULL;
void_type->u.f.parameters=NULL;

universal_type=(typeptr)malloc(1*sizeof(struct typeinfo));
universal_type->basetype=6;
universal_type->u.a.elemtype=NULL;
universal_type->u.f.st=NULL;
universal_type->u.f.returntype=NULL;
universal_type->u.f.parameters=NULL;


char_array_type=(typeptr)malloc(1*sizeof(struct typeinfo));
char_array_type->basetype=4;
char_array_type->u.f.st=NULL;
char_array_type->u.f.parameters=NULL;
char_array_type->u.a.elemtype=char_type;

paramlist print_parem2=(paramlist)malloc(1*sizeof(struct param));
print_parem2->type=universal_type;
print_parem2->next=NULL;

paramlist print_parem=(paramlist)malloc(1*sizeof(struct param));
print_parem->type=char_array_type;
print_parem->next=print_parem2;



print_type=(typeptr)malloc(1*sizeof(struct typeinfo));
print_type->basetype=5;
print_type->u.a.elemtype=NULL;
print_type->u.f.st=globals;
print_type->u.f.returntype=integer_type;
print_type->u.f.parameters=print_parem;


insert_st(globals,"printf",print_type,1);


getchar_type=(typeptr)malloc(1*sizeof(struct typeinfo));
getchar_type->basetype=5;
getchar_type->u.a.elemtype=NULL;
getchar_type->u.f.st=globals;
getchar_type->u.f.returntype=integer_type;
getchar_type->u.f.parameters=NULL;

insert_st(globals,"getchar",getchar_type,1);

}


int iserrortype(typeptr t)
{
	if(t!=NULL)
	{
		if(t->basetype==0)
			return 1;
		else return 0;
	}
	else
		{
		printf("type infomation is NULL\n");
		return 0;
		}
}


int isintegertype(typeptr t)
{
	if(t!=NULL)
	{
		if(t->basetype==1)
			return 1;
		else return 0;
	}
	else
		{
		printf("type infomation is NULL\n");
		return 0;
		}
}

int ischartype(typeptr t)
{
	if(t!=NULL)
	{
		if(t->basetype==2)
			return 1;
		else return 0;
	}
	else
		{
		printf("type infomation is NULL\n");
		return 0;
		}
}

int isstringtype(typeptr t)
{
	if(t!=NULL)
	{
		if(t->basetype==4)
			{
			if(iserrortype(compare_types(t->u.a.elemtype,char_type))!=1)
			return 1;
			}
		else return 0;
	}
	else
		{
		printf("type infomation is NULL\n");
		return 0;
		}
}

int isarraytype(typeptr t)
{
	if(t!=NULL)
	{
		if(t->basetype==4)
			return 1;
		else return 0;
	}
	else
		{
		printf("type infomation is NULL\n");
		return 0;
		}
}


int isfunctiontype(typeptr t)
{
	if(t!=NULL)
	{
		if(t->basetype==5)
			return 1;
		else return 0;
	}
	else
		{
		printf("type infomation is NULL\n");
		return 0;
		}
}

int istabletype(typeptr t)
{
	if(t!=NULL)
	{
		if(t->basetype==3)
			return 1;
		else return 0;
	}
	else
		{
		printf("type infomation is NULL\n");
		return 0;
		}
}

int isvoidtype(typeptr t)
{
	if(t!=NULL)
	{
		if(t->basetype==-1)
			return 1;
		else return 0;
	}
	else
		{
		printf("type infomation is NULL\n");
		return 0;
		}
}

