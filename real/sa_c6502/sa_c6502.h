#ifndef __SA_6502__
#define __SA_6502__

#define TRUE	1
#define FALSE	0

#define UWORD unsigned __int16
#define UBYTE unsigned char
#define WORD unsigned __int16
#define BYTE unsigned char
#define SBYTE char
#define SWORD signed __int16

#define NO_GOTO
#define NO_CYCLE_EXACT


void __declspec(dllexport) C6502_Initialise(BYTE* memory );
int __declspec(dllexport) C6502_JSR(WORD* adr, BYTE* areg, BYTE* xreg, BYTE* yreg, int* maxcycles);
void __declspec(dllexport) C6502_About(char** name, char** author, char** description );

#endif