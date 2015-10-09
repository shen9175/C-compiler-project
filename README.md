# C-compiler-project

self-educational purpose C compiler implementation project:
The project is implemented by standard C language.

implement partial C language functions with new C++11 standard for C part (no class and encapsulation)
char (8 bit) integer is implemented
integer type only implement 64bit long long type, no short(16bit), long(32bit) integer type, 
double 64bit is implement,
array and [] operator are implemented
pointer type is not implemented
basic built-in printf function without stdio library

basic local variable, global variable, +,-,*,/, ||,&&, +=, -=,*=, /=,()operators,
functional call, recursive.
the output code is assembly code on AMD-X64 architecture ISA on linux. 
No optimization process in code generation part.

The keyword parser part in file: clex.l is built with regular expression identification using flex
The grammar tree part in file: cgram.y is built with bison
semantic analysis, symbol table populate, type checking part is in file semantic.c
Three address code generation is in file tac.c
Final AMD-X64 ISA assembly code generation is in file finalcode.c

There is no low level optimization and high level optimization part in this compiler.

known issue:
global variable has some glitch
recursive function execution has some glitch




