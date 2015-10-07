#include "tac.h"
#include "symtab.h"


address getclosettableid(nodeptr t)
{
	if(t==NULL) return empty;
	if(t->nkids<=0) return empty;
	if(t->prodrule!=primary_expression&&t->prodrule!=identifier)
		getclosettableid(t->kids[0]);
	else
		{
			SymbolTableEntry ste=lookup_st(t->st,t->kids[0]->leaf->text);
			if(ste!=NULL)
				return ste->tableaddress;
			else return empty;
		}
}

int istable(nodeptr t)
{
	if(t==NULL) return 0;
	if(t->nkids<=0) return 0;

	if((t->prodrule==(postfix_expression+1)) && istabletype(t->kids[0]->type)==1)	//postfix_expression+LB+expression+RB
		return 1;
	else
		return istable(t->kids[0]);
	
}

address tablelhs(nodeptr t)
{
	if(t==NULL)
		return empty;
	if(t->nkids<=0)
		return empty;
	if(t->prodrule==postfix_expression+1)
		{
				address t2=newtemp(array_type,t->st);//t2 will hold the address, so it use array type for 8bytes
				address table,param;
				table.region=LOCAL;
				table.offset=0;
				param.region=IMM;
				param.offset=2;
				address zero;
				zero.offset=0;
				zero.region=IMM;
				address nulltable,yestable;
				nulltable.region=LABEL;
				nulltable.offset=0;
				nulltable.text="nulltable";
				yestable.region=LABEL;
				yestable.offset=0;
				nulltable.text="yestable";
				address t1=newtemp(integer_type,t->st);
				if(isintegertype(t->kids[2]->type))//integer
				{
					table.text="lookup_i";
					instrptr temp=t->code->tail->tail->tail->tail->tail;//skip original code
					t->code=concatecode(t->code->tail,newinstr(PARAMQ,t->kids[2]->place,empty,empty));
					t->code=concatecode(t->code,newinstr(PARAMQ,t->kids[0]->place,empty,empty));//check kids[0]->place if can hold table address
					t->code=concatecode(t->code,newinstr(CALL,t2,table,param));
					t->code=concatecode(t->code,newinstr(BEQ,nulltable,t2,zero));
					t->code=concatecode(t->code,newinstr(GOTO,yestable,empty,empty));
					t->code=concatecode(t->code,newinstr(LABEL,nulltable,empty,empty));
					t->code=concatecode(t->code,newinstr(PARAMQ,zero,empty,empty));	//if no key in the table, then insert the key with 0 string
					t->code=concatecode(t->code,newinstr(PARAML,t->kids[2]->place,empty,empty));
					t->code=concatecode(t->code,newinstr(PARAMQ,t->kids[0]->place,empty,empty));//check kids[0]->place if can hold table address
					table.text="insert_i";
					t->code=concatecode(t->code,newinstr(CALL,t1,table,param));
					t->code=concatecode(t->code,newinstr(PARAMQ,t->kids[2]->place,empty,empty));
					t->code=concatecode(t->code,newinstr(PARAMQ,t->kids[0]->place,empty,empty));//check kids[0]->place if can hold table address
					table.text="lookup_i";
					t->code=concatecode(t->code,newinstr(CALL,t2,table,param));//get the new key of the hash table entry address again
					t->code=concatecode(t->code,newinstr(LABEL,yestable,empty,empty));
					t->code=concatecode(t->code,temp);

				}
				else if(isstringtype(t->kids[2]->type))
				{
					table.text="lookup_s";
					instrptr temp=t->code->tail->tail->tail->tail->tail;//skip original code
					t->code=concatecode(t->code->tail,newinstr(PARAMQ,t->kids[2]->place,empty,empty));
					t->code=concatecode(t->code,newinstr(PARAMQ,t->kids[0]->place,empty,empty));//check kids[0]->place if can hold table address
					t->code=concatecode(t->code,newinstr(CALL,t2,table,param));
					t->code=concatecode(t->code,newinstr(BEQ,nulltable,t2,zero));
					t->code=concatecode(t->code,newinstr(GOTO,yestable,empty,empty));
					t->code=concatecode(t->code,newinstr(LABEL,nulltable,empty,empty));
					t->code=concatecode(t->code,newinstr(PARAMQ,zero,empty,empty));	//if no key in the table, then insert the key with 0 string
					t->code=concatecode(t->code,newinstr(PARAMQ,t->kids[2]->place,empty,empty));
					t->code=concatecode(t->code,newinstr(PARAMQ,t->kids[0]->place,empty,empty));//check kids[0]->place if can hold table address
					table.text="insert_s";
					t->code=concatecode(t->code,newinstr(CALL,t1,table,param));
					t->code=concatecode(t->code,newinstr(PARAMQ,t->kids[2]->place,empty,empty));
					t->code=concatecode(t->code,newinstr(PARAMQ,t->kids[0]->place,empty,empty));//check kids[0]->place if can hold table address
					table.text="lookup_s";
					t->code=concatecode(t->code,newinstr(CALL,t2,table,param));//get the new key of the hash table entry address again
					t->code=concatecode(t->code,newinstr(LABEL,yestable,empty,empty));
					t->code=concatecode(t->code,temp);

				}
				else fprintf(stderr,"Wrong index type in table expression!\n");

				return t2;
		}
	else
		{
				
				return tablelhs(t->kids[0]);
		}

}



int typesizecall(typeptr t)
{
	int size=0;

		switch(t->basetype)
				{
					case 1:	//integer
						size=4;	//4 bytes
						break;
					case 2: //char
						size=1;		//1 bytes
						break;
					case 3: //table
					case 4: //array,string
						size=8;	//8 bytes for 64-bit address: this is pointer
						break;
					case 5:	//function
						size=typesizecall(t->u.f.returntype);
						break;
					default:
						break;
				}
	return size;
}


void addparamnumbers(nodeptr t,int nparam)
{
	if(t==NULL) return;
	switch(t->prodrule)
		{
		case argument_expression_list:	//assignment_expression
			{
			instrptr p=t->code;
			while(p->src1.region!=STOP)
				p=p->tail;
			p->src1.region=IMM;
			p->src1.offset=nparam;
			break;
			}
		case argument_expression_list+1: //argument_expression_list+CM+assignment_expression
			{
				if(t->nkids>0)
				{
				addparamnumbers(t->kids[0],nparam);
				instrptr p=t->code;
				while(p->src1.region!=STOP)
				p=p->tail;
				p->src1.region=IMM;
				p->src1.offset=nparam;
				}
			break;
			}
		default:
			error("should not be here: addparamnumber()",t);
			break;
		}
}

address newtemp(struct typeinfo * t, SymbolTable st)
{
	newtempnumber++;
	char buffer[1024];
	sprintf(buffer,"tskldjfa%d",newtempnumber);
	boolSTE ste=insert_st(st,buffer,t,1);
	if (ste.correct==0)
		{
		fprintf(stderr,"Compiler internal error:Fail to asigne a different name for new temporary variable!\n");
		address r;
		r.region=0;
		r.offset=0;
		r.text=NULL;
		return r;
		}

	return ste.entry->place;
	
}

address newparamtemp(struct typeinfo * t, SymbolTable st)
{
	newtempnumber++;
	char buffer[64];
	sprintf(buffer,"t%d",newtempnumber);
	boolSTE ste=insert_st(st,buffer,t,1);
	if (ste.correct==0)
		{
		fprintf(stderr,"Compiler internal error:Fail to asigne a different name for new temporary variable!\n");
		address r;
		r.region=0;
		r.offset=0;
		r.text=NULL;
		return r;
		}

	return ste.entry->place;
	
}


instrptr newinstr(int optype,address destination,address arg1, address arg2)
{
	instrptr ninstr=(instrptr)malloc(1*sizeof(struct code));
	if (ninstr==NULL) fprintf(stderr,"Fail to allocate memory for new instruction!\n");

	ninstr->op=optype;
	ninstr->src1=arg1;
	ninstr->src2=arg2;
	ninstr->dest=destination;
	ninstr->head=ninstr;
	ninstr->tail=NULL;
	return ninstr;
}


address newlabel()
{
	newlabelnumber++;
	address r;
	r.region=LABEL;
	r.offset=newlabelnumber;
	r.text=NULL;
	return r;
}	

instrptr concatecode( instrptr front,  instrptr end)
{
	if(front==NULL)
	{
		if(end!=NULL)
			return end->head;//no matter end->head is NULL or not
		else
			return NULL;
	}
	else
	{
	if(front->head!=NULL)
		{
			if(end!=NULL)
			{
				instrptr p=front->head;
				while(p->tail!=NULL)
					p=p->tail;
				p->tail=end->head;
				return front->head;
			}
			else
				return front->head;
		}
	else
		{
			if(end!=NULL)
				return end->head;
			else
				return NULL;
		}
	}
}

void codeGen_bool(nodeptr t,address trueDst,address falseDst)
{

if(t==NULL)
	return;

if(t->nkids<=0)
	return;



switch(t->prodrule)
{
case relational_expression+1:		//relational_expression+LT+shift_expression
case relational_expression+2:		//relational_expression+GT+shift_expression
case relational_expression+3:		//relational_expression+LE+shift_expression
case relational_expression+4:		//relational_expression+GE+shift_expression
case equality_expression+1:		//equality_expression+EQ+relational_expression
case equality_expression+2:		//equality_expression+NE+relational_expression
case logical_and_expression+1:		//logical_and_expression+ANDAND+inclusive_or_expression
case logical_or_expression+1:		//logical_or_expression+OROR+logical_and_expression	
  {
	switch(t->kids[1]->leaf->category)
	{
		case LT:
		{
		t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
		t->code=concatecode(t->code,newinstr(BLT,trueDst,t->kids[0]->place,t->kids[2]->place));
		t->code=concatecode(t->code,newinstr(GOTO,falseDst,empty,empty));
		break;
		}
		case GT:
		{
		t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
		t->code=concatecode(t->code,newinstr(BGT,trueDst,t->kids[0]->place,t->kids[2]->place));
		t->code=concatecode(t->code,newinstr(GOTO,falseDst,empty,empty));
		break;
		}
		case LE:
		{
		t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
		t->code=concatecode(t->code,newinstr(BLE,trueDst,t->kids[0]->place,t->kids[2]->place));
		t->code=concatecode(t->code,newinstr(GOTO,falseDst,empty,empty));
		break;
		}
		case GE:
		{
		t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
		t->code=concatecode(t->code,newinstr(BGE,trueDst,t->kids[0]->place,t->kids[2]->place));
		t->code=concatecode(t->code,newinstr(GOTO,falseDst,empty,empty));
		break;
		}
		case EQ:
		{
		t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
		t->code=concatecode(t->code,newinstr(BEQ,trueDst,t->kids[0]->place,t->kids[2]->place));
		t->code=concatecode(t->code,newinstr(GOTO,falseDst,empty,empty));
		break;
		}
		case NE:
		{
		t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
		t->code=concatecode(t->code,newinstr(BNE,trueDst,t->kids[0]->place,t->kids[2]->place));
		t->code=concatecode(t->code,newinstr(GOTO,falseDst,empty,empty));
		break;
		}
		case ANDAND:
		{
			address L=newlabel();
			codeGen_bool(t->kids[0],L,falseDst);
			codeGen_bool(t->kids[2],trueDst,falseDst);
			t->code=concatecode(t->kids[0]->code,newinstr(LABEL,L,empty,empty));
			t->code=concatecode(t->code,t->kids[2]->code);
			break;
		}
		case OROR:
		{
			address L=newlabel();
			codeGen_bool(t->kids[0],trueDst,L);
			codeGen_bool(t->kids[2],trueDst,falseDst);
			t->code=concatecode(t->kids[0]->code,newinstr(LABEL,L,empty,empty));
			t->code=concatecode(t->code,t->kids[2]->code);
			break;
		}
		default:
			break;
	}
   break;
  }
  default:
	{
		int i;
		if(t->code==NULL)
		{
		for(i=0;i<t->nkids;i++)
			{
			codeGen_bool(t->kids[i],trueDst,falseDst);
			t->code = concatecode(t->code, t->kids[i]->code);
			}
		}
		break;
	}

	
}

}

void codeGen_break(nodeptr t ,address L_after)
{
	if(t==NULL)
		return;

	if(t->nkids<=0)
		return;

	switch(t->prodrule)
	{
		case jump_statement+2:			//BREAK+SM
			{
				if(t->nkids>0)
				{
					t->code->dest=L_after;//=newinstr(GOTO,L_after,empty,empty);
				}
			break;
			}
		default:
			{

				int i;
				for(i=0;i<t->nkids;i++)
					{
					codeGen_break(t->kids[i],L_after);
					}
			break;
			}
	}
	
}



void codeGen_continue(nodeptr t ,address L_top)
{
	if(t==NULL)
		return;

	if(t->nkids<=0)
		return;

	switch(t->prodrule)
	{
		case jump_statement+1:			//CONTINUE+SM
			{
				if(t->nkids>0)
				{
					t->code->dest=L_top;//t->code=newinstr(GOTO,L_top,empty,empty);
				}
			break;
			}
		default:
			{
				int i;
				for(i=0;i<t->nkids;i++)
				codeGen_continue(t->kids[i],L_top);
				break;
			}
			
	}


}




void codeGen_switch(nodeptr t, address t1, address L_after)
{
	if(t==NULL)
		return;

	if(t->nkids<=0)
		return;

	switch(t->prodrule)
	{

		case selection_statement+2:		//SWITCH+LP+expression+RP+statement
			{
			address t2=newtemp(t->kids[2]->type,t->st);
			t2=t->kids[2]->place;
			address L_after=newlabel();
			codeGen_break(t->kids[4],L_after);
			codeGen_switch(t->kids[4],t2,L_after);
			t->code=concatecode(t->kids[2]->code,t->kids[4]->code);
			t->code=concatecode(t->code,newinstr(LABEL,L_after,empty,empty));
			break;
			}
		case labeled_statement+1:		//CASE+constant_expression+COLON+statement
			{
			if(t->nkids>0)
			{
			t->code->src2=t1;
			}
			break;
			}
		default:
			{
				int i;
				for(i=0;i<t->nkids;i++)
					codeGen_switch(t->kids[i],t1,L_after);
			break;			
			}

	}	
}
void codegen(nodeptr t)
{
   int i;
   SymbolTableEntry ste,ste1;
   if (t==NULL) return;


   for(i=0;i<t->nkids;i++)
      codegen(t->kids[i]);

   switch (t->prodrule)
	{
		
		case identifier:	//IDENTIFIER
			{
			if(t->st==NULL)//global symbol table
				t->st=globals;	//find the symbol from global to local

			ste=check_id_all_scope(t->leaf->text,t->st);

			if(ste!=NULL)
				{
				t->place=ste->place;
				t->code=NULL;
				}
			else
				;//this is function prototype variable name. So it is not in the symbol table

			break;
			}
		case constant:		//ICON
			{
			//t->place=newtemp(integer_type);//??allocate memory for constant?
			t->place.region=IMM;
			t->place.offset=t->leaf->ival;
			t->code=NULL;
			break;
			}
		case constant+3:	//CCON
			{
			//t->place=newtemp(char_type);//??allocate memory for constant?
			t->place.region=IMM;
			t->place.offset=t->leaf->ival;
			t->code=NULL;
			break;
			}
		case primary_expression:	//IDENTIFIER
			{
			if(t->st==NULL)//global symbol table
				t->st=globals;	//find the symbol from global to local
			ste=check_id_all_scope(t->leaf->text,t->st);

			if(ste!=NULL)
				{
				t->place=ste->place;
				t->code=NULL;
				}
			else
				error("The compiler's typechecking has some problem",t);



			break;
			}
		case primary_expression+2:	//STRING_LITERAL
			{
			//t->place=newtemp(t->type,t->st);//the string constant will store the global static .data part when compile
			//address gstr;
			t->place.region=STR;
			t->place.offset=StrOffset;
			//t->code=newinstr(ADDR,t->place,gstr,empty);
			/*
			int j=StrOffset;
			int end=StrOffset;
			StrOffset+=strlen(t->leaf->sval)+1;
			globalstring=(char*)realloc(globalstring,StrOffset*sizeof(char));
			if(globalstring==NULL)
				error("realloc string error",t);
			for(;j<StrOffset;j++)
				globalstring[j]=t->leaf->sval[j-end];*/

			//here is for final code generation
			globalstring=(char**)realloc(globalstring,(StrOffset+1)*sizeof(char*));
			if(globalstring==NULL)
				error("realloc string error",t);
			globalstring[StrOffset]=strdup(t->leaf->text);
			StrOffset++;			
			break;
			}
		case primary_expression+3:	//LP+expression+RP
			{
			if(t->nkids>0)	//to avoid LP an RP come in this way which cause t doesn't have kids
			{
			t->place=t->kids[1]->place;
			t->code=t->kids[1]->code;
			}
			break;
			}
		case postfix_expression+1:	//postfix_expression+LB+expression+RB
			//if we have checked primary_expression, then we don't need to check the first postfix_expression. They all will go through primary_expression
			if(t->nkids>0)
			{
			address imm,t1,t2,t3;
			t1=newtemp(integer_type,t->st);//t1 will hold offset of subscript in index value, so it uses integer
			t2=newtemp(array_type,t->st);//t2 will hold the address, so it use array type for 8bytes
			//t3=newtemp(t->type,t->st);//t3 will hold the content of subscripting value. t->kids[0]->place hold the address of the array.
			typeptr typ;
			if(istabletype(t->kids[0]->type))
				{
					t3=newtemp(string_type,t->st);
					address table,param;
					table.region=LOCAL;
					table.offset=0;
					param.region=IMM;
					param.offset=2;
					if(isintegertype(t->kids[2]->type))//integer
					table.text="lookup_i";
					else if(isstringtype(t->kids[2]->type))
					table.text="lookup_s";
					else fprintf(stderr,"Wrong index type in table expression!\n");
					address tableaddress=getclosettableid(t->kids[0]);
					t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
					t->code=concatecode(t->code,newinstr(PARAMQ,t->kids[2]->place,empty,empty));
					t->code=concatecode(t->code,newinstr(PARAMQ,tableaddress,empty,empty));//check kids[0]->place if can hold table address
					t->code=concatecode(t->code,newinstr(CALL,t2,table,param));
					t->code=concatecode(t->code,newinstr(LCONT,t3,t2,empty));
					t->place=t3;
				}
			else
			{
			switch(t->type->basetype)//if this expression is table, then t->type is string_type,so use its kids[0] type
				{
					case 1:	//integer
						{
						imm.region=IMM;
						imm.offset=4;	//4 bytes for integer
						t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
						t->code=concatecode(t->code,newinstr(MUL,t1,t->kids[2]->place,imm));
						t->code=concatecode(t->code,newinstr(ADDR,t2,t->kids[0]->place,empty));
						t->code=concatecode(t->code,newinstr(ADD,t2,t1,t2));
						t->place=t2;
						//t->code=concatecode(t->code,newinstr(LCONT,t3,t2,empty));
						}
					break;
					case 2:	//char
						{
						imm.region=IMM;
						imm.offset=1;	//1 byte for char
						t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
						t->code=concatecode(t->code,newinstr(MUL,t1,t->kids[2]->place,imm));
						t->code=concatecode(t->code,newinstr(ADDR,t2,t->kids[0]->place,empty));
						t->code=concatecode(t->code,newinstr(ADD,t2,t1,t2));
						t->place=t2;
						//t->code=concatecode(t->code,newinstr(LCONT,t3,t2,empty));
						}
					break;
					case 4:	//array,string address
						{
						imm.region=IMM;
						imm.offset=8;	//8 bytes for address
						//t3=newtemp(integer_type,t->st);
						t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
						t->code=concatecode(t->code,newinstr(MUL,t1,t->kids[2]->place,imm));
						t->code=concatecode(t->code,newinstr(ADDR,t2,t->kids[0]->place,empty));
						t->code=concatecode(t->code,newinstr(ADD,t2,t1,t2));
						//t->code=concatecode(t->code,newinstr(LCONT,t3,t2,empty));
						t->place=t2;
						}
					break;
					default:
						error("Should not be here!\n",t);
						break;
				}
			}
			}
			break;
		case postfix_expression+2:	//postfix_expression+LP+argument_expression_list+RP
			if(t->nkids>0) //t->nkids=0 must be LP or RP
			{
				int bytesize=0;
				int nparam=0;
				paramlist p=t->kids[0]->type->u.f.parameters;
				while(p!=NULL)
				{
				//bytesize+=typesizecall(p->type);
				nparam++;
				p=p->next;}
				address param,t1;
				param.region=IMM;
				param.offset=nparam;
				addparamnumbers(t->kids[2],nparam);
				t1=newtemp(t->type,t->st);
				t->place=t1;//t->kids[0]->place;
				t->code=concatecode(t->kids[2]->code,newinstr(CALL,t1,t->kids[0]->place,param));//t->place is for t->place.text: function name, not for return value memory location; param is for the number of parameters
			}
			break;
		case postfix_expression+3:	//postfix_expression+LP+RP
			if(t->nkids>0) //t->nkids=0 must be LP or RP
			{
				address param,t1;
				param.region=IMM;
				param.offset=0;
				t1=newtemp(t->type,t->st);
				t->place=t1;//t->kids[0]->place;
				t->code=newinstr(CALL,t1,t->kids[0]->place,param);//t->place is for t->place.text: function name, not for return value memory location; param is for the number of parameters				
			}
			break;

		

		case argument_expression_list:	//assignment_expression
			{
			address stop;
			stop.region=STOP;
			stop.offset=0;
			t->place=t->kids[0]->place;
			if(typesizecall(t->type)==8)
				{
				address t1=newtemp(array_type,t->st);
				if(isstringtype(t->type)==0)
					{
					t->code=concatecode(t->kids[0]->code,newinstr(ADDR,t1,t->place,empty));
					t->code=concatecode(t->code,newinstr(PARAMQ,t1,stop,empty));
					}
				else
					t->code=concatecode(t->kids[0]->code,newinstr(PARAMQ,t->place,stop,empty));
				}
			else if(typesizecall(t->type)==1)
				{
				t->code=concatecode(t->kids[0]->code,newinstr(PARAMB,t->place,stop,empty));
				}
			else if(typesizecall(t->type)==4)
				{
				t->code=concatecode(t->kids[0]->code,newinstr(PARAML,t->place,stop,empty));
				}
			else;
			break;
			}
		case argument_expression_list+1: //argument_expression_list+CM+assignment_expression
			{
			if(t->nkids>0)
			{
			address stop;
			stop.region=STOP;
			stop.offset=0;
				t->place=t->kids[2]->place;
				if(typesizecall(t->type)==8)
					{
					address t1=newtemp(array_type,t->st);
					//t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
					if(isstringtype(t->type)==0)
						{
						t->code=concatecode(t->code,t->kids[2]->code);
						t->code=concatecode(t->code,newinstr(ADDR,t1,t->place,empty));
						t->code=concatecode(t->code,newinstr(PARAMQ,t1,stop,empty));
						t->code=concatecode(t->code,t->kids[0]->code);//reverse push parameters
						}
					else
					//t->code=concatecode(t->code,newinstr(PARAMQ,t->place,stop,empty));
					t->code=concatecode(t->code,t->kids[2]->code);
					t->code=concatecode(newinstr(PARAMQ,t->place,stop,empty),t->kids[0]->code);
					}
				else if(typesizecall(t->type)==1)
					{
					//t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
					t->code=concatecode(t->code,t->kids[2]->code);
					t->code=concatecode(t->code,newinstr(PARAMB,t->place,stop,empty));
					t->code=concatecode(t->code,t->kids[0]->code);//reverse push parameters
					}
				else if(typesizecall(t->type)==4)
					{
					//t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
					t->code=concatecode(t->code,t->kids[2]->code);
					t->code=concatecode(t->code,newinstr(PARAML,t->place,stop,empty));
					t->code=concatecode(t->code,t->kids[0]->code);//reverse push parameters
					}
				else;	
			}			
			break;
			}
		case unary_expression+3:	//unary_operator+cast_expression
			{
			if(t->kids[0]->leaf->category==MINUS)
				{
				t->place=newtemp(t->type,t->st);
				t->code=concatecode(t->kids[1]->code,newinstr(NEG,t->place,t->kids[1]->place,empty));
				}
			else
			{error("ct does not support this type of unary operator",t);t->code=NULL;}
			}
			break;
		//case unary_operator: omit all here

		case multiplicative_expression+1:	//multiplicative_expression+MUL+cast_expression
			if(t->nkids>0)
			{
			t->place=newtemp(t->type,t->st);
			t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
			t->code=concatecode(t->code,newinstr(MUL,t->place,t->kids[0]->place,t->kids[2]->place));
			}
			break;
		case multiplicative_expression+2:	//multiplicative_expression+DIV+cast_expression
			if(t->nkids>0)
			{
			t->place=newtemp(t->type,t->st);
			t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
			t->code=concatecode(t->code,newinstr(DIV,t->place,t->kids[0]->place,t->kids[2]->place));
			}
			break;

		case additive_expression+1:		//additive_expression+PLUS+multiplicative_expression
			if(t->nkids>0)
			{
			t->place=newtemp(t->type,t->st);
			t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
			t->code=concatecode(t->code,newinstr(ADD,t->place,t->kids[0]->place,t->kids[2]->place));
			}
			break;
		case additive_expression+2:		//additive_expression+MINUS+multiplicative_expression
			if(t->nkids>0)
			{
			t->place=newtemp(t->type,t->st);
			t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
			t->code=concatecode(t->code,newinstr(SUB,t->place,t->kids[0]->place,t->kids[2]->place));
			}
			break;
		case primary_expression+1:		//constant
		case postfix_expression:		//primary_expression
		case unary_expression:			//postfix_expression
		case cast_expression:			//unary_expression
		case multiplicative_expression:		//cast_expression
		case additive_expression:		//multiplicative_expression
		case shift_expression:			//additive_expression
		case relational_expression:		//shift_expression
		case equality_expression:		//relational_expression
		case and_expression:			//equality_expression
		case exclusive_or_expression:		//and_expression
		case inclusive_or_expression:		//exclusive_or_expression
		case logical_and_expression:		//inclusive_or_expression
		case logical_or_expression:		//logical_and_expression
		case conditional_expression:		//logical_or_expression
		case assignment_expression:		//conditional_expression
		case expression:			//assignment_expression
		case constant_expression:		//conditional_expression
			{
			t->place=t->kids[0]->place;
			t->code=t->kids[0]->code;
			break;
			}
		case identifier+1:			//ENUMERATION_CONSTANT
		case constant+1:			//FCON
		case constant+2:			//ENUMERATION_CONSTANT
		case primary_expression+4:		//generic_selection
		case postfix_expression+4:		//postfix_expression+DOT+identifier	
		case postfix_expression+5:		//postfix_expression+FOLLOW+identifier
		case postfix_expression+6:		//postfix_expression+INCOP	
		case postfix_expression+7:		//postfix_expression+DECOP	
		case postfix_expression+8:		//LP+type_name+RP+LC+initializer_list+RC	
		case postfix_expression+9:		//LP+type_name+RP+LC+initializer_list+CM+RC
		case unary_expression+1:		//INCOP unary_expression		
		case unary_expression+2:		//DECOP unary_expression		
		case unary_expression+4:		//SIZEOF+unary_expression
		case unary_expression+5:		//SIZEOF+LP+type_name+RP
		case unary_expression+6:		//_ALIGNOF+LP+type_name+RP
		case cast_expression+1:			//LP type_name RP cast_expression
		case multiplicative_expression+3:	//multiplicative_expression+MOD+cast_expression
		case shift_expression+1:		//shift_expression+SHL+additive_expression
		case shift_expression+2:		//shift_expression+SHR+additive_expression
		case and_expression+1:			//and_expression+AND+equality_expression
		case exclusive_or_expression+1:		//exclusive_or_expression+ER+and_expression
		case inclusive_or_expression+1:		//inclusive_or_expression+OR+exclusive_or_expression
		case conditional_expression+1:		//logical_or_expression+QUEST+expression+COLON+conditional_expression
			{
			//error("ct does not support",t);
			t->code=NULL;
			break;
			}

		case relational_expression+1:		//relational_expression+LT+shift_expression
		case relational_expression+2:		//relational_expression+GT+shift_expression
		case relational_expression+3:		//relational_expression+LE+shift_expression
		case relational_expression+4:		//relational_expression+GE+shift_expression
		case equality_expression+1:		//equality_expression+EQ+relational_expression
		case equality_expression+2:		//equality_expression+NE+relational_expression
		case logical_and_expression+1:		//logical_and_expression+ANDAND+inclusive_or_expression
		case logical_or_expression+1:		//logical_or_expression+OROR+logical_and_expression
			{
			break;
			}

		case assignment_expression+1:		//assignment_expression+assignment_operator+conditional_expression
			{
			if(t->kids[1]->leaf->category==ASN)
			{
			if(istable(t->kids[0]))
				t->place=tablelhs(t->kids[0]);
			else
				t->place=t->kids[0]->place;
			t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
			t->code=concatecode(t->code,newinstr(ASSN,t->place,t->kids[2]->place,empty));

			}
			else
				{error("ct does not support complex assignment operator",t);t->code=NULL;}
			break;
			}

		case expression+1:			//assignment_expression+CM+assignment_expression
			{
			if(t->nkids>0)
			{
			t->place=t->kids[2]->place;
			t->code=concatecode(t->kids[0]->code,t->kids[2]->code);
			}
			break;
			}
 
		case statement:				//labeled_statement
		case statement+1:			//compound_statement
		case statement+2:			//expression_statement
		case statement+3:			//selection_statement
		case statement+4:			//iteration_statement
		case statement+5:			//jump_statement
			{
			t->code=t->kids[0]->code;
			break;
			}

		case labeled_statement:			//identifier+COLON+statement
			{
			if(t->nkids>0)
			{
			address L;
			L.region=LABEL;
			L.offset=0;
			L.text=strdup(t->kids[0]->leaf->text);
			t->code=concatecode(newinstr(LABEL,L,empty,empty),t->kids[2]->code);
			}
			break;
			}
		case labeled_statement+1:		//CASE+constant_expression+COLON+statement
			{
			address L1=newlabel();
			t->code=concatecode(newinstr(BEQ,L1,t->kids[1]->place,empty),newinstr(LABEL,L1,empty,empty));
			t->code=concatecode(t->code,t->kids[3]->code);
			break;
			}
		case labeled_statement+2:		//DEFAULT+COLON+statement
			{
			t->code=t->kids[2]->code;
			break;
			}

		case compound_statement:		//LC+block_item_list+RC
			{
			if(t->nkids>0)
			{
			t->code=t->kids[1]->code;
			}
			break;
			}
		case compound_statement+1:		//LC+RC
			{
			t->code=NULL;
			break;
			}

		case block_item_list:			//block_item
			{
			t->code=t->kids[0]->code;
			break;
			}
		case block_item_list+1:			//block_item_list+block_item
			{
			t->code=concatecode(t->kids[0]->code,t->kids[1]->code);
			break;
			}

		case block_item:			//declaration
			{
			t->code=NULL;
			break;
			}
		case block_item+1:			//statement
			{
			t->code=t->kids[0]->code;
			break;
			}


		case expression_statement:		//expression+SM
			{
			if(t->nkids>0)
			{
			t->place=t->kids[0]->place;
			t->code=t->kids[0]->code;
			}
			break;
			}
		case expression_statement+1:		//SM
			{
			t->code=NULL;
			break;
			}
		
		case selection_statement:		//IF+LP+expression+RP+statement
			{
				if(t->nkids>0)
				{
				address L_then=newlabel();
				address L_after=newlabel();
				codeGen_bool(t->kids[2],L_then,L_after);
				t->code=concatecode(t->kids[2]->code,newinstr(LABEL,L_then,empty,empty));
				t->code=concatecode(t->code,t->kids[4]->code);
				t->code=concatecode(t->code,newinstr(LABEL,L_after,empty,empty));
				}
				break;
			}
		case selection_statement+1:		//IF+LP+expression+RP+statement+ELSE+statement
			{
				if(t->nkids>0)
				{
				address L_then=newlabel();
				address L_else=newlabel();
				address L_after=newlabel();
				codeGen_bool(t->kids[2],L_then,L_else);
				t->code=concatecode(t->kids[2]->code,newinstr(LABEL,L_then,empty,empty));
				t->code=concatecode(t->code,t->kids[4]->code);
				t->code=concatecode(t->code,newinstr(GOTO,L_after,empty,empty));
				t->code=concatecode(t->code,newinstr(LABEL,L_else,empty,empty));
				t->code=concatecode(t->code,t->kids[6]->code);
				t->code=concatecode(t->code,newinstr(LABEL,L_after,empty,empty));
				}
			break;
			}
		case selection_statement+2:		//SWITCH+LP+expression+RP+statement
			{
			if(t->nkids>0)
			{
			address t1=newtemp(t->kids[2]->type,t->st);
			t1=t->kids[2]->place;
			address L_after=newlabel();
			codeGen_break(t->kids[4],L_after);
			codeGen_switch(t->kids[4],t1,L_after);
			t->code=concatecode(t->kids[2]->code,t->kids[4]->code);
			t->code=concatecode(t->code,newinstr(LABEL,L_after,empty,empty));
			}
			break;
			}

		case iteration_statement:		//WHILE+LP+expression+RP+statement
			{
				if(t->nkids>0)
				{
				address L_top=newlabel();
				address L_body=newlabel();
				address L_after=newlabel();
				codeGen_break(t->kids[4],L_after);
				codeGen_continue(t->kids[4],L_top);
				codeGen_bool(t->kids[2],L_body,L_after);
				t->code=concatecode(newinstr(LABEL,L_top,empty,empty),t->kids[2]->code);
				t->code=concatecode(t->code,newinstr(LABEL,L_body,empty,empty));
				t->code=concatecode(t->code,t->kids[4]->code);
				t->code=concatecode(t->code,newinstr(GOTO,L_top,empty,empty));
				t->code=concatecode(t->code,newinstr(LABEL,L_after,empty,empty));
				}
				break;
			}
		case iteration_statement+1:		//DO+statement+WHILE+LP+expression+RP+SM
			{
				if(t->nkids>0)
				{
				address L_body=newlabel();
				address L_after=newlabel();
				codeGen_break(t->kids[1],L_after);
				codeGen_continue(t->kids[1],L_body);
				codeGen_bool(t->kids[4],L_body,L_after);
				t->code=concatecode(newinstr(LABEL,L_body,empty,empty),t->kids[1]->code);
				t->code=concatecode(t->code,t->kids[4]->code);
				t->code=concatecode(t->code,newinstr(LABEL,L_after,empty,empty));
				}
				break;
			}
		case iteration_statement+2:		//FOR+LP+expression+SM+expression+SM+expression+RP+statement
			{
				if(t->nkids>0)
				{
				address L_top=newlabel();
				address L_body=newlabel();
				address L_after=newlabel();
				codeGen_break(t->kids[8],L_after);
				codeGen_continue(t->kids[8],L_top);
				codeGen_bool(t->kids[4],L_body,L_after);
				t->code=concatecode(t->kids[2]->code,newinstr(LABEL,L_top,empty,empty));
				t->code=concatecode(t->code,t->kids[4]->code);
				t->code=concatecode(t->code,t->kids[8]->code);
				t->code=concatecode(t->code,t->kids[6]->code);
				t->code=concatecode(t->code,newinstr(GOTO,L_top,empty,empty));
				t->code=concatecode(t->code,newinstr(LABEL,L_after,empty,empty));
				}
				break;
			}
		case iteration_statement+3:		//FOR+LP+SM+expression+SM+expression+RP+statement
			{
				if(t->nkids>0)
				{
				address L_top=newlabel();
				address L_body=newlabel();
				address L_after=newlabel();
				codeGen_break(t->kids[7],L_after);
				codeGen_continue(t->kids[7],L_top);
				codeGen_bool(t->kids[3],L_body,L_after);
				t->code=concatecode(newinstr(LABEL,L_top,empty,empty),t->kids[3]->code);
				t->code=concatecode(t->code,t->kids[7]->code);
				t->code=concatecode(t->code,t->kids[5]->code);
				t->code=concatecode(t->code,newinstr(GOTO,L_top,empty,empty));
				t->code=concatecode(t->code,newinstr(LABEL,L_after,empty,empty));
				}
				break;
			}
		case iteration_statement+4:		//FOR+LP+expression+SM+SM+expression+RP+statement
			{
				if(t->nkids>0)
				{
				address L_top=newlabel();
				address L_after=newlabel();
				codeGen_break(t->kids[7],L_after);
				codeGen_continue(t->kids[7],L_top);
				t->code=concatecode(t->kids[2]->code,newinstr(LABEL,L_top,empty,empty));
				t->code=concatecode(t->code,t->kids[7]->code);
				t->code=concatecode(t->code,t->kids[5]->code);
				t->code=concatecode(t->code,newinstr(GOTO,L_top,empty,empty));
				t->code=concatecode(t->code,newinstr(LABEL,L_after,empty,empty));
				}
				break;
			}
		case iteration_statement+5:		//FOR+LP+expression+SM+expression+SM+RP+statement
			{
				if(t->nkids>0)
				{
				address L_top=newlabel();
				address L_body=newlabel();
				address L_after=newlabel();
				codeGen_break(t->kids[7],L_after);
				codeGen_continue(t->kids[7],L_top);
				codeGen_bool(t->kids[4],L_body,L_after);
				t->code=concatecode(t->kids[2]->code,newinstr(LABEL,L_top,empty,empty));
				t->code=concatecode(t->code,t->kids[4]->code);
				t->code=concatecode(t->code,t->kids[7]->code);
				t->code=concatecode(t->code,newinstr(GOTO,L_top,empty,empty));
				t->code=concatecode(t->code,newinstr(LABEL,L_after,empty,empty));
				}
				break;
			}
		case iteration_statement+6:		//FOR+LP+SM+SM+expression+RP+statement
			{
				if(t->nkids>0)
				{
				address L_top=newlabel();
				address L_after=newlabel();
				codeGen_break(t->kids[6],L_after);
				codeGen_continue(t->kids[6],L_top);
				t->code=concatecode(newinstr(LABEL,L_top,empty,empty),t->kids[6]->code);
				t->code=concatecode(t->code,t->kids[4]->code);
				t->code=concatecode(t->code,newinstr(GOTO,L_top,empty,empty));
				t->code=concatecode(t->code,newinstr(LABEL,L_after,empty,empty));
				}
				break;
			}
		case iteration_statement+7:		//FOR+LP+SM+expression+SM+RP+statement
			{
				if(t->nkids>0)
				{
				address L_top=newlabel();
				address L_body=newlabel();
				address L_after=newlabel();
				codeGen_break(t->kids[6],L_after);
				codeGen_continue(t->kids[6],L_top);
				codeGen_bool(t->kids[3],L_body,L_after);
				t->code=concatecode(newinstr(LABEL,L_top,empty,empty),t->kids[3]->code);
				t->code=concatecode(t->code,t->kids[6]->code);
				t->code=concatecode(t->code,newinstr(GOTO,L_top,empty,empty));
				t->code=concatecode(t->code,newinstr(LABEL,L_after,empty,empty));
				}
				break;
			}
		case iteration_statement+8:		//FOR+LP+expression+SM+SM+RP+statement
			{
				if(t->nkids>0)
				{
				address L_top=newlabel();
				address L_after=newlabel();
				codeGen_break(t->kids[6],L_after);
				codeGen_continue(t->kids[6],L_top);
				t->code=concatecode(t->kids[2]->code,newinstr(LABEL,L_top,empty,empty));
				t->code=concatecode(t->code,t->kids[6]->code);
				t->code=concatecode(t->code,newinstr(GOTO,L_top,empty,empty));
				t->code=concatecode(t->code,newinstr(LABEL,L_after,empty,empty));
				}
				break;
			}
		case iteration_statement+9:		//FOR+LP+SM+SM+RP+statement
			{
				if(t->nkids>0)
				{
				address L_top=newlabel();
				address L_after=newlabel();
				codeGen_break(t->kids[5],L_after);
				codeGen_continue(t->kids[5],L_top);
				t->code=concatecode(newinstr(LABEL,L_top,empty,empty),t->kids[5]->code);
				t->code=concatecode(t->code,newinstr(GOTO,L_top,empty,empty));
				t->code=concatecode(t->code,newinstr(LABEL,L_after,empty,empty));
				}
				break;
			}
		case iteration_statement+10:		//FOR+LP+declaration+expression+SM+expression+RP+statement
		case iteration_statement+11:		//FOR+LP+declaration+SM+expression+RP+statement
		case iteration_statement+12:		//FOR+LP+declaration+expression+SM+RP+statement
		case iteration_statement+13:		//FOR+LP+declaration+SM+RP+statement
			{
				if(t->nkids>0)
				{
				error("ct does not support C99-C11 standard: declaration in the FOR loop",t);
				}
				break;
			}

		case jump_statement:			//GOTO+identifier+SM
			{
				if(t->nkids>0)
				{
				address id;
				id.region=LABEL;
				id.offset=0;
				id.text=strdup(t->kids[1]->leaf->text);
				t->code=newinstr(GOTO,id,empty,empty);
				}
				break;
			}
		case jump_statement+1:			//CONTINUE+SM
			{
				t->code=newinstr(GOTO,empty,empty,empty);//reserve the code place, then when get laber number run the break/continue_codegen to patch the L_after or L_top
				break;
				break;
			}
		case jump_statement+2:			//BREAK+SM
			{
				t->code=newinstr(GOTO,empty,empty,empty);//reserve the code place, then when get laber number run the break/continue_codegen to patch the L_after or L_top
				break;
			}
		case jump_statement+3:			//RETURN+expression+SM
			{
				if(t->nkids>0)
				{
					t->code=concatecode(t->kids[1]->code,newinstr(RETV,t->kids[1]->place,empty,empty));
				}
				break;
			}
		case jump_statement+4:			//RETURN+SM
			{
				if(t->nkids>0)
				{
					t->code=newinstr(RET,empty,empty,empty);
				}
				break;
			}
		case function_definition+1:		//declaration_specifiers+declarator+compound_statement
			{
				char* s;
				address f,six;
				six.region=IMM;
				s=get_funcname(t->kids[1]);
				SymbolTableEntry ste;
				ste=lookup_st(globals,s);//look up the function definition in the global symbol table.
				if(ste==NULL)
					error("should not be here:in function definition, we cannot find the name corresponding function",t);
				paramlist p=ste->type->u.f.parameters;
				int n=0,sum=0;
				while(p!=NULL)
					{
					if(n<=5)
						{
						six.six[n]=typesizecall(p->type);
						sum+=six.six[n];
						}
					n++;//calculate how many parameters does this function has.
					p=p->next;
					} 


				six.offset=n;
				f.region=LABEL;
				f.offset=calculateoffset(t->kids[2]->st)+sum;//calculate total local variable's bytes and first six parameters which are allocated on the top of the locals
				f.text=strdup(s);
				instrptr q=t->kids[2]->code;
				while(q->tail!=NULL)
					q=q->tail;
				if(q->op!=RET&&q->op!=RETV)
					t->kids[2]->code=concatecode(t->kids[2]->code,newinstr(RET,empty,empty,empty));
				t->code=concatecode(newinstr(FUNC,f,six,empty),t->kids[2]->code);
				t->code=concatecode(t->code,newinstr(END,f,empty,empty));
			break;
			}
		default:
			{
			for(i=0;i<t->nkids;i++)
				t->code = concatecode(t->code, t->kids[i]->code);
			break;
			}
	}
}


