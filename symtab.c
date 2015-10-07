#include "symtab.h"



SymbolTable new_st(int nbuckets,int scope)
{
	SymbolTable n=(SymbolTable)malloc(1*sizeof(struct sym_table));
	n->tblist=(struct sym_entry**)malloc(nbuckets*sizeof(struct sym_entry*));
	n->nBuckets=nbuckets;
	n->nEntries=0;
	n->scope=scope;
	return n;
}


void delete_st(SymbolTable st)
{
	SymbolTableEntry p,prev;
	int i;

	for(i=0;i<st->nBuckets;i++)
	{
		p=st->tblist[i];
		while(p!=NULL)
		{
			prev=p;
			p=p->next;
			free(prev->s);
			prev->s=NULL;
			free(prev);
			prev=NULL;
		}

	}

	free(st->tblist);
	st->tblist=NULL;
	free(st);
	st=NULL;
}

int hash(SymbolTable st,char*s)
{
	register int h=0;
	register char c;
	while(c=*s++)
	{
		h+=c&0377;
		h*=37;
	}
	if(h<0)
		h=-h;
	return h%st->nBuckets;
}




boolSTE insert_st(SymbolTable st, char*s, typeptr t, int prototype)
{
	register int i;
	boolSTE r;
	SymbolTableEntry p;
	int l;

	int h=hash(st,s);
	p=st->tblist[h];
	while(p!=NULL)
	{
		if(strcmp(s,p->s)==0) //p->s is in the list
			{
			if(p->prototype==0)//if it's function definition in the list
				{
				r.entry=NULL;	//return error, or return true
				r.correct=0;
				return r;
				}
			else	{
				r.entry=p;
				r.correct=0;
				return r;
				}
			}
		p=p->next;
	}
	address temp;
	temp.region=st->scope;
	temp.offset=calculateoffset(st)+typesize(t);//here +typesize(t) is because first local should be first -size(%rbp)
	temp.text=strdup(s);
//here is add information for param symbols: this is for x86_64 passing parameter convention
	if(temp.region==ARG)
	{
	temp.ordernumber=calculatesymbols(st)+1;
		if(temp.ordernumber==6)
		{
			st->firstsixsize=temp.offset;
		}
		if(temp.ordernumber>6)
		{
			temp.seven=temp.offset-typesize(t)-st->firstsixsize;//totalbytes_from_the_7th_to_before_current_one
		}
	}
	p=(SymbolTableEntry)malloc(1*sizeof(struct sym_entry));
	p->next=st->tblist[h];
	st->tblist[h]=p;
	p->table=st;
	p->s=strdup(s);
	p->prototype=prototype;
	p->type=t;
	p->place=temp;
	/*
	p->place.region=st->scope;
	p->place.offset=calculateoffset(st);
	p->place.text=strdup(s);*/
	st->nEntries++;
	r.entry=p;
	r.correct=1;
	return r;
}

int calculatesymbols(SymbolTable st)
{
	int i;
	int number=0;
	SymbolTableEntry p;
	for(i=0;i<st->nBuckets;i++)
	{
		p=st->tblist[i];
		while(p!=NULL)
			{
				number++;
				p=p->next;
			}	
	}

	return number;
}
int calculateoffset(SymbolTable st)
{
	int i;
	int offset=0;
	SymbolTableEntry p;
	for(i=0;i<st->nBuckets;i++)
	{
		p=st->tblist[i];
		while(p!=NULL)
			{
				offset+=typesize(p->type);//calculate type size recursively
				p=p->next;
			}	
	}

	return offset;
}

int typesize(typeptr t)
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
						size=8;	//8 bytes for 64-bit address
						break;
					case 4: //array,string
						{
						if(t->u.a.size!=0)
							size=t->u.a.size*typesize(t->u.a.elemtype);//this is static array: allocate the memory
						else
						size=8;	//8 bytes for 64-bit address: this is pointer
						}
						break;
					case 5:	//function
						size=typesize(t->u.f.returntype);
						break;
					default:
						break;
				}
	return size;
}


SymbolTableEntry lookup_st(SymbolTable st, char*s)
{
	register int i;
	int h;
	SymbolTableEntry p;

	h=hash(st,s);
	p=st->tblist[h];
	while(p!=NULL)
	{
		if(strcmp(s,p->s)==0)
			return p;
		p=p->next;
	}
	return NULL;
}



