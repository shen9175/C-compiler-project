#include "symtab.h"
#include "tac.h"

char buffer[1024];
int localbytes;
int lfbcount;
void finalcode(nodeptr t)
{

	int i;
//print global variables

	SymbolTableEntry ste;
	for(i=0;i<globals->nBuckets;i++)
	{
		ste=globals->tblist[i];
		while(ste!=NULL)
			{
				if(isfunctiontype(ste->type)!=1)//not function so it is a variable
				{
					fprintf(yyout,"%s:\n\n",ste->s);
				}
				ste=ste->next;
			}
	}
	fprintf(yyout,"\t.section\t.rodata\n");

//print string constants

	for(i=0;i<StrOffset;i++)
	{
		fprintf(yyout,".LC%d:\n",i);
		fprintf(yyout,"\t.string %s\n",globalstring[i]);
	}
	fprintf(yyout,"\t.text\n");




	int nparms=0;
	instrptr p=t->code;
	while(p!=NULL)
	{
		switch(p->op)
		{
			case ADD:
			{
				fprintf(yyout,"\tmovq\t%s, %%rax\n",realaddress(p->src1));
				fprintf(yyout,"\taddq\t%s, %%rax\n",realaddress(p->src2));
				fprintf(yyout,"\tmovq\t%%rax, %s\n",realaddress(p->dest));
			break;				
			}
			case SUB:
			{
				fprintf(yyout,"\tmovq\t%s, %%rax\n",realaddress(p->src1));
				fprintf(yyout,"\tsubq\t%s, %%rax\n",realaddress(p->src2));
				fprintf(yyout,"\tmovq\t%%rax, %s\n",realaddress(p->dest));
			break;
			}
			case MUL:
			{
				fprintf(yyout,"\tmovq\t%s, %%rax\n",realaddress(p->src1));
				fprintf(yyout,"\timulq\t%s, %%rax\n",realaddress(p->src2));
				fprintf(yyout,"\tmovq\t%%rax, %s\n",realaddress(p->dest));
			break;
			}
			case DIV:
			{
				fprintf(yyout,"\tmovq\t$0, %%rdx\n");
				fprintf(yyout,"\tmovq\t%s, %%rax\n",realaddress(p->src1));
				fprintf(yyout,"\tidivq\t%s\n",realaddress(p->src2));
				fprintf(yyout,"\tmovq\t%%rax, %s\n",realaddress(p->dest));
			break;
			}
			case NEG:
			{
				fprintf(yyout,"\tmovq\t%s, %%rax\n",realaddress(p->src1));
				fprintf(yyout,"\tnegq\t%%rax\n");
				fprintf(yyout,"\tmovq\t%%rax, %s\n",realaddress(p->dest));
			break;
			}
			case LABEL:
			{
				fprintf(yyout,"%s:\n",realaddress(p->dest));
			break;
			}
			case GOTO:
			{
				fprintf(yyout,"\tjmp\t%s\n",realaddress(p->dest));
			break;
			}
			case RET:
			{
				fprintf(yyout,"\tleave\n");
				fprintf(yyout,"\tret\n");
			break;
			}
			case RETV:
			{
				fprintf(yyout,"\tmovq\t%s, %%rax\n",realaddress(p->dest));
				fprintf(yyout,"\tleave\n");
				fprintf(yyout,"\tret\n");
			break;
			}
			case ASSN:
			{
				fprintf(yyout,"\tmovq\t%s, %%rax\n",realaddress(p->src1));
				fprintf(yyout,"\tmovq\t%%rax, %s\n",realaddress(p->dest));
			break;
			}
			case BLT:
			{
				fprintf(yyout,"\tmovq\t%s, %%rax\n",realaddress(p->src1));
				fprintf(yyout,"\tcmpq\t%s, %%rax\n",realaddress(p->src2));
				fprintf(yyout,"\tjl\t%s\n",realaddress(p->dest));
			break;
			}
			case BGT:
			{
				fprintf(yyout,"\tmovq\t%s, %%rax\n",realaddress(p->src1));
				fprintf(yyout,"\tcmpq\t%s, %%rax\n" ,realaddress(p->src2));
				fprintf(yyout,"\tjg\t%s\n",realaddress(p->dest));				
			break;	
			}
			case BLE:
			{
				fprintf(yyout,"\tmovq\t%s, %%rax\n",realaddress(p->src1));
				fprintf(yyout,"\tcmpq\t%s, %%rax\n",realaddress(p->src2));
				fprintf(yyout,"\tjle\t%s\n",realaddress(p->dest));					
			break;	
			}
			case BGE:
			{
				fprintf(yyout,"\tmovq\t%s, %%rax\n",realaddress(p->src1));
				fprintf(yyout,"\tcmpq\t%s, %%rax\n",realaddress(p->src2));
				fprintf(yyout,"\tjge\t%s\n",realaddress(p->dest));					
			break;	
			}
			case BNE:
			{
				fprintf(yyout,"\tmovq\t%s, %%rax\n",realaddress(p->src1));
				fprintf(yyout,"\tcmpq\t%s, %%rax\n",realaddress(p->src2));
				fprintf(yyout,"\tjne\t%s\n",realaddress(p->dest));					
			break;	
			}
			case BEQ:
			{
				fprintf(yyout,"\tmovq\t%s, %%rax\n",realaddress(p->src1));
				fprintf(yyout,"\tcmpq\t%s, %%rax\n",realaddress(p->src2));
				fprintf(yyout,"\tje\t%s\n",realaddress(p->dest));					
			break;	
			}
			case PARAMB:
			{

				nparms++;
				if(nparms<=6)
				{
					switch (p->src1.offset-nparms+1)//p->src1.offset is holding the total number of parameters of this function.
						{
							case 1:
								fprintf(yyout,"\tmovb\t%s, %%dil\n",realaddress(p->dest));
								break;
							case 2:
								fprintf(yyout,"\tmovb\t%s, %%sil\n",realaddress(p->dest));
								break;
							case 3:
								fprintf(yyout,"\tmovb\t%s, %%dl\n",realaddress(p->dest));
								break;
							case 4:
								fprintf(yyout,"\tmovb\t%s, %%cl\n",realaddress(p->dest));
								break;
							case 5:
								fprintf(yyout,"\tmovb\t%s, %%r8b\n",realaddress(p->dest));
								break;
							case 6:
								fprintf(yyout,"\tmovb\t%s, %%r9b\n",realaddress(p->dest));
								break;
						}
				}
				else
				{
						fprintf(yyout,"\tpushb\t%s\n",realaddress(p->dest));
				}
	
			break;	
			}
			case PARAML:
			{

				nparms++;
				if(nparms<=6)
				{
					switch (p->src1.offset-nparms+1)//p->src1.offset is holding the total number of parameters of this function.
						{
							case 1:
								fprintf(yyout,"\tmovl\t%s, %%edi\n",realaddress(p->dest));
								break;
							case 2:
								fprintf(yyout,"\tmovl\t%s, %%esi\n",realaddress(p->dest));
								break;
							case 3:
								fprintf(yyout,"\tmovl\t%s, %%edx\n",realaddress(p->dest));
								break;
							case 4:
								fprintf(yyout,"\tmovl\t%s, %%ecx\n",realaddress(p->dest));
								break;
							case 5:
								fprintf(yyout,"\tmovl\t%s, %%r8d\n",realaddress(p->dest));
								break;
							case 6:
								fprintf(yyout,"\tmovl\t%s, %%r9d\n",realaddress(p->dest));
								break;
						}
				}
				else
				{
						fprintf(yyout,"\tpushl\t%s\n",realaddress(p->dest));
				}
	
			break;	
			}
			case PARAMQ:
			{
				nparms++;
				if(nparms<=6)
				{
					switch (p->src1.offset-nparms+1)
						{
							case 1:
								fprintf(yyout,"\tmovq\t%s, %%rdi\n",realaddress(p->dest));
								break;
							case 2:
								fprintf(yyout,"\tmovq\t%s, %%rsi\n",realaddress(p->dest));
								break;
							case 3:
								fprintf(yyout,"\tmovq\t%s, %%rdx\n",realaddress(p->dest));
								break;
							case 4:
								fprintf(yyout,"\tmovq\t%s, %%rcx\n",realaddress(p->dest));
								break;
							case 5:
								fprintf(yyout,"\tmovq\t%s, %%r8\n",realaddress(p->dest));
								break;
							case 6:
								fprintf(yyout,"\tmovq\t%s, %%r9\n",realaddress(p->dest));
								break;
						}
				}
				else
				{
						fprintf(yyout,"\tpushq\t%s\n",realaddress(p->dest));
				}
			
			break;	
			}
			case CALL:
			{
				fprintf(yyout,"\tmovq\t$0, %%rax\n");//for no variable argument for call printf
				fprintf(yyout,"\tcall\t%s\n",p->src1.text);
				fprintf(yyout,"\tmovq\t%%rax, %s\n",realaddress(p->dest));
				nparms=0;
			break;	
			}
			case ADDR:
			{
				fprintf(yyout,"\tleaq\t%s, %%rax\n",realaddress(p->src1));
				fprintf(yyout,"\tmovq\t%%rax, %s\n",realaddress(p->dest));				
			break;	
			}
			case LCONT:
			{
				fprintf(yyout,"\tmovq\t%s, %%rax\n",realaddress(p->src1));
				fprintf(yyout,"\tmovq\t(%%rax), %s\n",realaddress(p->dest));				
			break;	
			}
			case FUNC:
			{
				fprintf(yyout,".globl %s\n",p->dest.text);
				fprintf(yyout,"\t.type\t%s, @function\n",p->dest.text);
				fprintf(yyout,"%s:\n",p->dest.text);
				fprintf(yyout,".LFB%d:\n",lfbcount);
				fprintf(yyout,"\tpushq\t%%rbp\n");
				fprintf(yyout,"\tmovq\t%%rsp, %%rbp\n");//change frame
				fprintf(yyout,"\tsubq\t$%d, %%rsp\n",p->dest.offset);//allocate local variables and first six parameters
				localbytes=p->dest.offset;
				//p->src1.offset contains the total number of parameters of this function
				//p->src1.six[6] contains first 6 parameters' each size bytes
				//save all first 6 passing parameters from parameter special registers to memory locations
			int high;
				for(i=0;i<p->src1.offset;i++)
				{
				switch(i)
				{
					case 0:
						{
							switch(p->src1.six[0])//1,4,8
							{
							case 1:
								high=localbytes+1;
								fprintf(yyout,"\tmovb\t%%dil, -%d(%%rbp)\n",high);
								break;
							case 4:
								high=localbytes+4;
								fprintf(yyout,"\tmovl\t%%edi, -%d(%%rbp)\n",high);
								break;
							case 8:
								high=localbytes+8;
								fprintf(yyout,"\tmovq\t%%rdi, -%d(%%rbp)\n",high);
								break;
								default: fprintf(stderr,"should not be here!\n");
								break;
							}
						}
						break;
					case 1:
					{
							switch(p->src1.six[1])//1,4,8
							{
							case 1:
								high+=1;
								fprintf(yyout,"\tmovb\t%%sil, -%d(%%rbp)\n",high);
								break;
							case 4:
								high+=4;
								fprintf(yyout,"\tmovl\t%%esi, -%d(%%rbp)\n",high);
								break;
							case 8:
								high+=8;
								fprintf(yyout,"\tmovq\t%%rsi, -%d(%%rbp)\n",high);
								break;
							default: fprintf(stderr,"should not be here!\n");
								break;
							}
					}
					break;
					case 2:
					{
							switch(p->src1.six[2])//1,4,8
							{
							case 1:
								high+=1;
								fprintf(yyout,"\tmovb\t%%dl, -%d(%%rbp)\n",high);
								break;
							case 4:
								high+=4;
								fprintf(yyout,"\tmovl\t%%edx, -%d(%%rbp)\n",high);
								break;
							case 8:
								high+=8;
								fprintf(yyout,"\tmovq\t%%rdx, -%d(%%rbp)\n",high);
								break;
							default: fprintf(stderr,"should not be here!\n");
								break;
							}
					}
					break;
					case 3:
					{
							switch(p->src1.six[3])//1,4,8
							{
							case 1:
								high+=1;
								fprintf(yyout,"\tmovb\t%%cl, -%d(%%rbp)\n",high);
								break;
							case 4:
								high+=4;
								fprintf(yyout,"\tmovl\t%%ecx, -%d(%%rbp)\n",high);
								break;
							case 8:
								high+=8;
								fprintf(yyout,"\tmovq\t%%rcx, -%d(%%rbp)\n",high);
								break;
							default: fprintf(stderr,"should not be here!\n");
								break;
							}
					}
					break;
					case 4:
					{
							switch(p->src1.six[4])//1,4,8
							{
							case 1:
								high+=1;
								fprintf(yyout,"\tmovb\t%%r8b, -%d(%%rbp)\n",high);
								break;
							case 4:
								high+=4;
								fprintf(yyout,"\tmovl\t%%r8d, -%d(%%rbp)\n",high);
								break;
							case 8:
								high+=8;
								fprintf(yyout,"\tmovq\t%%r8, -%d(%%rbp)\n",high);
								break;
							default: fprintf(stderr,"should not be here!\n");
								break;
							}
					}
					break;
					case 5:
					{
							switch(p->src1.six[4])//1,4,8
							{
							case 1:
								high+=1;
								fprintf(yyout,"\tmovb\t%%r9b, -%d(%%rbp)\n",high);
								break;
							case 4:
								high+=4;
								fprintf(yyout,"\tmovl\t%%r9d, -%d(%%rbp)\n",high);
								break;
							case 8:
								high+=8;
								fprintf(yyout,"\tmovq\t%%r9, -%d(%%rbp)\n",high);
								break;
							default: fprintf(stderr,"should not be here!\n");
								break;
							}
					}
					break;
					default:
					break;
				}//switch
				}//for


			break;
			}
			case END:
			{
				fprintf(yyout,".LFE%d:\n",lfbcount);
				lfbcount++;
			break;
			}
			default:
			{
				fprintf(stderr,"undefined opcode\n");
				break;
			}
		}
		p=p->tail;
	}
}

char* realaddress(address place)
{
	switch(place.region)
		{
			case IMM:
				{
					sprintf(buffer,"$%d",place.offset);
					break;
				}
			case LOCAL:
				{
					sprintf(buffer,"-%d(%%rbp)",place.offset);
					break;
				}
			case LABEL:
				{
					if(place.text!=NULL)
						sprintf(buffer,"%s",place.text);
					else
						sprintf(buffer,".L%d",place.offset);
					break;
				}
			case ARG:
				{
					if(place.ordernumber<=6)
					{
						sprintf(buffer,"-%d(%%rbp)",(localbytes+place.offset));
					}
					else if(place.ordernumber==7)
					{
						sprintf(buffer,"8(%%rbp)");
					}
					else
					{
						sprintf(buffer,"%d(%%rbp)",(8+place.seven));
					}
					break;
				}
			case GLOBAL:
				{
					sprintf(buffer,"%s(%%rip)",place.text);
					break;
				}
			case STR:
				{
					sprintf(buffer,"$.LC%d",place.offset);
					break;
				}
		}
	return buffer;
}
