#ifndef __SA_6502__
#define __SA_6502__

#include <stdint.h>

#define TRUE    1
#define FALSE   0

#define UWORD   uint16_t
#define UBYTE   uint8_t
#define WORD    uint16_t
#define BYTE    uint8_t
#define SBYTE   char
#define SWORD   int16_t

void __declspec(dllexport) C6502_Initialise(BYTE* memory );
int __declspec(dllexport) C6502_JSR(WORD* adr, BYTE* areg, BYTE* xreg, BYTE* yreg, int* maxcycles);
void __declspec(dllexport) C6502_About(char** name, char** author, char** description );

#endif
