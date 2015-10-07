

ct: main.o finalcode.o tac.o semantic.o symtab.o cgram.tab.o lex.yy.o
	gcc -g -o ct main.o finalcode.o tac.o semantic.o symtab.o cgram.tab.o lex.yy.o -lm

finalcode.o: finalcode.c tac.h symtab.h
	gcc -g -c finalcode.c

tac.o: tac.c tac.h symtab.h
	gcc -g -c tac.c

main.o: main.c symtab.h cgram.tab.h
	gcc -g -c main.c

semantic.o: semantic.c symtab.h
	gcc -g -c semantic.c

symtab.o: symtab.c symtab.h
	gcc -g -c symtab.c

cgram.tab.o: cgram.tab.c symtab.h
	gcc -g -c -DYYDEBUG cgram.tab.c

lex.yy.o: lex.yy.c symtab.h
	gcc -g -c lex.yy.c

cgram.tab.c: cgram.y symtab.h
	bison -d -v cgram.y

cgram.tab.h: cgram.tab.c

lex.yy.c: clex.l cgram.tab.h parsetree.h
	flex clex.l



turnin:
	tar -cvf hw6.tar  makefile main.c clex.l cgram.y symtab.h symtab.c semantic.c parsetree.h tac.h tac.c finalcode.c
