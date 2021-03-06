/* Crippled version for the sa_c6502.dll use only */

#include "sa_c6502.h"

#ifndef __CPU_INCLUDED__
#define __CPU_INCLUDED__

//! #include "atari.h"

#define N_FLAG 0x80
#define V_FLAG 0x40
#define G_FLAG 0x20
#define B_FLAG 0x10
#define D_FLAG 0x08
#define I_FLAG 0x04
#define Z_FLAG 0x02
#define C_FLAG 0x01

void CPU_GetStatus(void);
void CPU_PutStatus(void);
void CPU_Reset(void);
void NMI(void);

#define GenerateIRQ() (IRQ = 1)

#ifdef CPUASS
extern void CPU_INIT(void);
extern void CPUGET(void);       /* put from CCR, N & Z FLAG into regP */
extern void CPUPUT(void);       /* put from regP into CCR, N & Z FLAG */
#endif

/*
extern UWORD regPC;
extern UBYTE regA;
extern UBYTE regP;
extern UBYTE regS;
extern UBYTE regY;
extern UBYTE regX;
*/

#define SetN regP|=N_FLAG
#define ClrN regP&=(~N_FLAG)
#define SetV regP|=V_FLAG
#define ClrV regP&=(~V_FLAG)
#define SetB regP|=B_FLAG
#define ClrB regP&=(~B_FLAG)
#define SetD regP|=D_FLAG
#define ClrD regP&=(~D_FLAG)
#define SetI regP|=I_FLAG
#define ClrI regP&=(~I_FLAG)
#define SetZ regP|=Z_FLAG
#define ClrZ regP&=(~Z_FLAG)
#define SetC regP|=C_FLAG
#define ClrC regP&=(~C_FLAG)

//extern UBYTE IRQ;

/*
#define REMEMBER_PC_STEPS 16
extern UWORD remember_PC[REMEMBER_PC_STEPS];

#define REMEMBER_JMP_STEPS 16
extern UWORD remember_JMP[REMEMBER_JMP_STEPS];
*/

#endif
