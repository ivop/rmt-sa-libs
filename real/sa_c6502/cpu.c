/* Crippled version for the sa_c6502.dll use only */
/*  CPU.C  */
/*
 BUG: In zeropage indirect mode, address can be fetched from 0x00ff and 0x0100.
 */

#include <stdio.h>
#include <stdlib.h> /* for exit() */
#include <string.h> /* for memmove() */

#include "sa_c6502.h"       // our sa_c6502 pridavky hook ;)

UBYTE* g_memory = NULL;
int xpos=0;

#define dGetByte(adr)       g_memory[adr]
#define GetByte(adr)        g_memory[adr]
#define dPutByte(adr,data)  g_memory[adr]=data
#define PutByte(adr,data)   g_memory[adr]=data

#define dGetWordAligned(adr) ( *(WORD*)(g_memory+adr) )
#define dGetWord(adr)        ( *(WORD*)(g_memory+adr) )

#define SA_C6502_INIT    { PC=*adr; A=*areg; X=*xreg; Y=*yreg; regP=0x34; S=0xff; }
#define SA_C6502_RETURN  { *adr=PC; *areg=A; *xreg=X; *yreg=Y; *maxcycles=(*maxcycles-xpos); return insn; }

void __declspec(dllexport) C6502_Initialise(BYTE* memory)
{
    g_memory = memory;
}

#include "cpu.h"

#define PL dGetByte(0x0100 + ++S)
#define PH(x) dPutByte(0x0100 + S--, x)

#define RMW_GetByte(x,addr) x = GetByte(addr);

#define PHW(x) PH((x)>>8); PH((x) & 0xff)

UBYTE regP;                     //* Processor Status Byte (Partial) * /

/*
   ===============================================================
   Z flag: This actually contains the result of an operation which
   would modify the Z flag. The value is tested for
   equality by the BEQ and BNE instruction.
   ===============================================================
 */


#define AND(t_data) data = t_data; Z = N = A &= data
#define CMP(t_data) data = t_data; Z = N = A - data; C = (A >= data)
#define CPX(t_data) data = t_data; Z = N = X - data; C = (X >= data)
#define CPY(t_data) data = t_data; Z = N = Y - data; C = (Y >= data)
#define EOR(t_data) data = t_data; Z = N = A ^= data
#define LDA(data) Z = N = A = data
#define LDX(data) Z = N = X = data
#define LDY(data) Z = N = Y = data
#define ORA(t_data) data = t_data; Z = N = A |= data

#define PHP(x)  data = (N & 0x80); \
                data |= V ? 0x40 : 0; \
                data |= (regP & x); \
                data |= (Z == 0) ? 0x02 : 0; \
                data |= C; \
                PH(data);

#define PHPB0 PHP(0x2c)
#define PHPB1 PHP(0x3c)

#define PLP data = PL; \
            N = (data & 0x80); \
            V = (data & 0x40) ? 1 : 0; \
            Z = (data & 0x02) ? 0 : 1; \
            C = (data & 0x01); \
            regP = (data & 0x3c) | 0x30;


/*  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
int cycles[256] =
{
    7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,     /* 0x */
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,     /* 1x */
    6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,     /* 2x */
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,     /* 3x */

    6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,     /* 4x */
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,     /* 5x */
    6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,     /* 6x */
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,     /* 7x */

    2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,     /* 8x */
    2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,     /* 9x */
    2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,     /* Ax */
    2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,     /* Bx */

    2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,     /* Cx */
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,     /* Dx */
    2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,     /* Ex */
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7      /* Fx */
};

/* decrement 1 or 2 cycles for conditional jumps */
#define BRANCH(cond)    if (cond) {         \
        SWORD sdata = (SBYTE)dGetByte(PC++);\
        if ( (sdata + (UBYTE)PC) & 0xff00)  \
            xpos++;                         \
        xpos++;                             \
        PC += sdata;                        \
        break;                                \
    }                                       \
    PC++;                                   \
    break;
/* decrement 1 cycle for X (or Y) index overflow */
#define NCYCLES_Y       if ( (UBYTE) addr < Y ) xpos++;
#define NCYCLES_X       if ( (UBYTE) addr < X ) xpos++;


int __declspec(dllexport) C6502_JSR(WORD* adr, BYTE* areg, BYTE* xreg, BYTE* yreg, int* maxcycles)
{
    UBYTE insn;

#define OPCODE(code)    case 0x##code:

    UBYTE N;                    //* bit7 zero (0) or bit 7 non-zero (1) * /
    UBYTE Z;                    //* zero (0) or non-zero (1) * /
    UBYTE V;
    UBYTE C;                    //* zero (0) or one(1) * /

    UWORD PC;
    UBYTE S;
    UBYTE A;
    UBYTE X;
    UBYTE Y;

    UWORD addr;
    UBYTE data;


    if (!g_memory) return -1;
    
    SA_C6502_INIT;

// Addressing modes

#define ABSOLUTE    addr=dGetWord(PC);PC+=2;
#define ZPAGE       addr=dGetByte(PC++);
#define ABSOLUTE_X  addr=dGetWord(PC)+X;PC+=2;
#define ABSOLUTE_Y  addr=dGetWord(PC)+Y;PC+=2;
#define INDIRECT_X  addr=(UBYTE)(dGetByte(PC++)+X);addr=dGetWord(addr);
#define INDIRECT_Y  addr=dGetByte(PC++);addr=dGetWord(addr)+Y;
#define ZPAGE_X     addr=(UBYTE)(dGetByte(PC++)+X);
#define ZPAGE_Y     addr=(UBYTE)(dGetByte(PC++)+Y);

    xpos = 0;
    while (xpos < *maxcycles) {
        insn = dGetByte(PC++);
        xpos += cycles[insn];

    switch (insn) {

    OPCODE(00)              /* BRK */
        {
            SA_C6502_RETURN;
        }
        break;

    OPCODE(01)              /* ORA (ab,x) */
        INDIRECT_X;
        ORA(GetByte(addr));
        break;

    OPCODE(03)              /* ASO (ab,x) [unofficial - ASL then ORA with Acc] */
        INDIRECT_X;

    aso:
        RMW_GetByte(data, addr);
        C = (data & 0x80) ? 1 : 0;
        data <<= 1;
        PutByte(addr, data); 
        Z = N = A |= data;
        break;

    OPCODE(04)              /* NOP ab [unofficial - skip byte] */
    OPCODE(44)
    OPCODE(64)
    OPCODE(14)              /* NOP ab,x [unofficial - skip byte] */
    OPCODE(34)
    OPCODE(54)
    OPCODE(74)
    OPCODE(d4)
    OPCODE(f4)
    OPCODE(80)              /* NOP #ab [unofficial - skip byte] */
    OPCODE(82)
    OPCODE(89)
    OPCODE(c2)
    OPCODE(e2)
        PC++;
        break;

    OPCODE(05)              /* ORA ab */
        ZPAGE;
        ORA(dGetByte(addr));
        break;

    OPCODE(06)              /* ASL ab */
        ZPAGE;
        data = dGetByte(addr);
        C = (data & 0x80) ? 1 : 0;
        Z = N = data << 1;
        dPutByte(addr, Z);
        break;

    OPCODE(07)              /* ASO zpage [unofficial - ASL then ORA with Acc] */
        ZPAGE;

    aso_zpage:
        data = dGetByte(addr);
        C = (data & 0x80) ? 1 : 0;
        data <<= 1;
        dPutByte(addr, data); 
        Z = N = A |= data;
        break;

    OPCODE(08)              /* PHP */
        PHPB1;
        break;

    OPCODE(09)              /* ORA #ab */
        ORA(dGetByte(PC++));
        break;

    OPCODE(0a)              /* ASL */
        C = (A & 0x80) ? 1 : 0;
        Z = N = A <<= 1;
        break;

    OPCODE(0b)              /* ANC #ab [unofficial - AND then copy N to C (Fox) */
    OPCODE(2b)
        AND(dGetByte(PC++));
        C = N >= 0x80;
        break;

    OPCODE(0c)              /* NOP abcd [unofficial - skip word] */
        PC += 2;
        break;

    OPCODE(0d)              /* ORA abcd */
        ABSOLUTE;
        ORA(GetByte(addr));
        break;

    OPCODE(0e)              /* ASL abcd */
        ABSOLUTE;
        RMW_GetByte(data, addr);
        C = (data & 0x80) ? 1 : 0;
        Z = N = data << 1;
        PutByte(addr, Z);
        break;

    OPCODE(0f)              /* ASO abcd [unofficial - ASL then ORA with Acc] */
        ABSOLUTE;
        goto aso;

    OPCODE(10)              /* BPL */
        BRANCH(!(N & 0x80))

    OPCODE(11)              /* ORA (ab),y */
        INDIRECT_Y;
        NCYCLES_Y;
        ORA(GetByte(addr));
        break;

    OPCODE(13)              /* ASO (ab),y [unofficial - ASL then ORA with Acc] */
        INDIRECT_Y;
        goto aso;

    OPCODE(15)              /* ORA ab,x */
        ZPAGE_X;
        ORA(dGetByte(addr));
        break;

    OPCODE(16)              /* ASL ab,x */
        ZPAGE_X;
        data = dGetByte(addr);
        C = (data & 0x80) ? 1 : 0;
        Z = N = data << 1;
        dPutByte(addr, Z);
        break;

    OPCODE(17)              /* ASO zpage,x [unofficial - ASL then ORA with Acc] */
        ZPAGE_X;
        goto aso_zpage;

    OPCODE(18)              /* CLC */
        C = 0;
        break;

    OPCODE(19)              /* ORA abcd,y */
        ABSOLUTE_Y;
        NCYCLES_Y;
        ORA(GetByte(addr));
        break;

    OPCODE(1b)              /* ASO abcd,y [unofficial - ASL then ORA with Acc] */
        ABSOLUTE_Y;
        goto aso;

    OPCODE(1c)              /* NOP abcd,x [unofficial - skip word] */
    OPCODE(3c)
    OPCODE(5c)
    OPCODE(7c)
    OPCODE(dc)
    OPCODE(fc)
        if (dGetByte(PC) + X >= 0x100)
            xpos++;
        PC += 2;
        break;

    OPCODE(1d)              /* ORA abcd,x */
        ABSOLUTE_X;
        NCYCLES_X;
        ORA(GetByte(addr));
        break;

    OPCODE(1e)              /* ASL abcd,x */
        ABSOLUTE_X;
        RMW_GetByte(data, addr);
        C = (data & 0x80) ? 1 : 0;
        Z = N = data << 1;
        PutByte(addr, Z);
        break;

    OPCODE(1f)              /* ASO abcd,x [unofficial - ASL then ORA with Acc] */
        ABSOLUTE_X;
        goto aso;

    OPCODE(20)              /* JSR abcd */
        {
            UWORD retadr = PC + 1;
            PHW(retadr);
            PC = dGetWord(PC);
        }
        break;

    OPCODE(21)              /* AND (ab,x) */
        INDIRECT_X;
        AND(GetByte(addr));
        break;

    OPCODE(23)              /* RLA (ab,x) [unofficial - ROL Mem, then AND with A] */
        INDIRECT_X;

    rla:
        RMW_GetByte(data, addr);
        if (C) {
            C = (data & 0x80) ? 1 : 0;
            data = (data << 1) | 1;
        }
        else {
            C = (data & 0x80) ? 1 : 0;
            data = (data << 1);
        }
        PutByte(addr, data);
        Z = N = A &= data;
        break;

    OPCODE(24)              /* BIT ab */
        ZPAGE;
        N = dGetByte(addr);
        V = N & 0x40;
        Z = (A & N);
        break;

    OPCODE(25)              /* AND ab */
        ZPAGE;
        AND(dGetByte(addr));
        break;

    OPCODE(26)              /* ROL ab */
        ZPAGE;
        data = dGetByte(addr);
        Z = N = (data << 1) | C;
        C = (data & 0x80) ? 1 : 0;
        dPutByte(addr, Z);
        break;

    OPCODE(27)              /* RLA zpage [unofficial - ROL Mem, then AND with A] */
        ZPAGE;

    rla_zpage:
        data = dGetByte(addr);
        if (C) {
            C = (data & 0x80) ? 1 : 0;
            data = (data << 1) | 1;
        }
        else {
            C = (data & 0x80) ? 1 : 0;
            data = (data << 1);
        }
        dPutByte(addr, data);
        Z = N = A &= data;
        break;

    OPCODE(28)              /* PLP */
        PLP;
        /*! CPUCHECKIRQ;*/
        break;

    OPCODE(29)              /* AND #ab */
        AND(dGetByte(PC++));
        break;

    OPCODE(2a)              /* ROL */
        Z = N = (A << 1) | C;
        C = (A & 0x80) ? 1 : 0;
        A = Z;
        break;

    OPCODE(2c)              /* BIT abcd */
        ABSOLUTE;
        N = GetByte(addr);
        V = N & 0x40;
        Z = (A & N);
        break;

    OPCODE(2d)              /* AND abcd */
        ABSOLUTE;
        AND(GetByte(addr));
        break;

    OPCODE(2e)              /* ROL abcd */
        ABSOLUTE;
        RMW_GetByte(data, addr);
        Z = N = (data << 1) | C;
        C = (data & 0x80) ? 1 : 0;
        PutByte(addr, Z);
        break;

    OPCODE(2f)              /* RLA abcd [unofficial - ROL Mem, then AND with A] */
        ABSOLUTE;
        goto rla;

    OPCODE(30)              /* BMI */
        BRANCH(N & 0x80)

    OPCODE(31)              /* AND (ab),y */
        INDIRECT_Y;
        NCYCLES_Y;
        AND(GetByte(addr));
        break;

    OPCODE(33)              /* RLA (ab),y [unofficial - ROL Mem, then AND with A] */
        INDIRECT_Y;
        goto rla;

    OPCODE(35)              /* AND ab,x */
        ZPAGE_X;
        AND(dGetByte(addr));
        break;

    OPCODE(36)              /* ROL ab,x */
        ZPAGE_X;
        data = dGetByte(addr);
        Z = N = (data << 1) | C;
        C = (data & 0x80) ? 1 : 0;
        dPutByte(addr, Z);
        break;

    OPCODE(37)              /* RLA zpage,x [unofficial - ROL Mem, then AND with A] */
        ZPAGE_X;
        goto rla_zpage;

    OPCODE(38)              /* SEC */
        C = 1;
        break;

    OPCODE(39)              /* AND abcd,y */
        ABSOLUTE_Y;
        NCYCLES_Y;
        AND(GetByte(addr));
        break;

    OPCODE(3b)              /* RLA abcd,y [unofficial - ROL Mem, then AND with A] */
        ABSOLUTE_Y;
        goto rla;

    OPCODE(3d)              /* AND abcd,x */
        ABSOLUTE_X;
        NCYCLES_X;
        AND(GetByte(addr));
        break;

    OPCODE(3e)              /* ROL abcd,x */
        ABSOLUTE_X;
        RMW_GetByte(data, addr);
        Z = N = (data << 1) | C;
        C = (data & 0x80) ? 1 : 0;
        PutByte(addr, Z);
        break;

    OPCODE(3f)              /* RLA abcd,x [unofficial - ROL Mem, then AND with A] */
        ABSOLUTE_X;
        goto rla;

    OPCODE(40)              /* RTI */
        PLP;
        data = PL;
        PC = (PL << 8) | data;
        /*! CPUCHECKIRQ;*/
        break;

    OPCODE(41)              /* EOR (ab,x) */
        INDIRECT_X;
        EOR(GetByte(addr));
        break;

    OPCODE(43)              /* LSE (ab,x) [unofficial - LSR then EOR result with A] */
        INDIRECT_X;

    lse:
        RMW_GetByte(data, addr);
        C = data & 1;
        data >>= 1;
        PutByte(addr, data);
        Z = N = A ^= data;
        break;

    OPCODE(45)              /* EOR ab */
        ZPAGE;
        EOR(dGetByte(addr));
        break;

    OPCODE(46)              /* LSR ab */
        ZPAGE;
        data = dGetByte(addr);
        C = data & 1;
        Z = data >> 1;
        N = 0;
        dPutByte(addr, Z);
        break;

    OPCODE(47)              /* LSE zpage [unofficial - LSR then EOR result with A] */
        ZPAGE;

    lse_zpage:
        data = dGetByte(addr);
        C = data & 1;
        data = data >> 1;
        dPutByte(addr, data);
        Z = N = A ^= data;
        break;

    OPCODE(48)              /* PHA */
        PH(A);
        break;

    OPCODE(49)              /* EOR #ab */
        EOR(dGetByte(PC++));
        break;

    OPCODE(4a)              /* LSR */
        C = A & 1;
        Z = N = A >>= 1;
        break;

    OPCODE(4b)              /* ALR #ab [unofficial - Acc AND Data, LSR result] */
        data = A & dGetByte(PC++);
        C = data & 1;
        Z = N = A = (data >> 1);
        break;

    OPCODE(4c)              /* JMP abcd */
        PC = dGetWord(PC);
        break;

    OPCODE(4d)              /* EOR abcd */
        ABSOLUTE;
        EOR(GetByte(addr));
        break;

    OPCODE(4e)              /* LSR abcd */
        ABSOLUTE;
        RMW_GetByte(data, addr);
        C = data & 1;
        Z = data >> 1;
        N = 0;
        PutByte(addr, Z);
        break;

    OPCODE(4f)              /* LSE abcd [unofficial - LSR then EOR result with A] */
        ABSOLUTE;
        goto lse;

    OPCODE(50)              /* BVC */
        BRANCH(!V)

    OPCODE(51)              /* EOR (ab),y */
        INDIRECT_Y;
        NCYCLES_Y;
        EOR(GetByte(addr));
        break;

    OPCODE(53)              /* LSE (ab),y [unofficial - LSR then EOR result with A] */
        INDIRECT_Y;
        goto lse;

    OPCODE(55)              /* EOR ab,x */
        ZPAGE_X;
        EOR(dGetByte(addr));
        break;

    OPCODE(56)              /* LSR ab,x */
        ZPAGE_X;
        data = dGetByte(addr);
        C = data & 1;
        Z = data >> 1;
        N = 0;
        dPutByte(addr, Z);
        break;

    OPCODE(57)              /* LSE zpage,x [unofficial - LSR then EOR result with A] */
        ZPAGE_X;
        goto lse_zpage;

    OPCODE(58)              /* CLI */
        ClrI;
        /*! CPUCHECKIRQ;*/
        break;

    OPCODE(59)              /* EOR abcd,y */
        ABSOLUTE_Y;
        NCYCLES_Y;
        EOR(GetByte(addr));
        break;

    OPCODE(5b)              /* LSE abcd,y [unofficial - LSR then EOR result with A] */
        ABSOLUTE_Y;
        goto lse;

    OPCODE(5d)              /* EOR abcd,x */
        ABSOLUTE_X;
        NCYCLES_X;
        EOR(GetByte(addr));
        break;

    OPCODE(5e)              /* LSR abcd,x */
        ABSOLUTE_X;
        RMW_GetByte(data, addr);
        C = data & 1;
        Z = data >> 1;
        N = 0;
        PutByte(addr, Z);
        break;

    OPCODE(5f)              /* LSE abcd,x [unofficial - LSR then EOR result with A] */
        ABSOLUTE_X;
        goto lse;

    OPCODE(60)              /* RTS */

        if (S=0xff) SA_C6502_RETURN;

        data = PL;
        PC = ((PL << 8) | data) + 1;
        break;

    OPCODE(61)              /* ADC (ab,x) */
        INDIRECT_X;
        data = GetByte(addr);
        goto adc;

    OPCODE(63)              /* RRA (ab,x) [unofficial - ROR Mem, then ADC to Acc] */
        INDIRECT_X;

    rra:
        RMW_GetByte(data, addr);
        if (C) {
            C = data & 1;
            data = (data >> 1) | 0x80;
        }
        else {
            C = data & 1;
            data >>= 1;
        }
        PutByte(addr, data);
        goto adc;

    OPCODE(65)              /* ADC ab */
        ZPAGE;
        data = dGetByte(addr);
        goto adc;

    OPCODE(66)              /* ROR ab */
        ZPAGE;
        data = dGetByte(addr);
        Z = N = (C << 7) | (data >> 1);
        C = data & 1;
        dPutByte(addr, Z);
        break;

    OPCODE(67)              /* RRA zpage [unofficial - ROR Mem, then ADC to Acc] */
        ZPAGE;

    rra_zpage:
        data = dGetByte(addr);
        if (C) {
            C = data & 1;
            data = (data >> 1) | 0x80;
        }
        else {
            C = data & 1;
            data >>= 1;
        }
        dPutByte(addr, data);
        goto adc;

    OPCODE(68)              /* PLA */
        Z = N = A = PL;
        break;

    OPCODE(69)              /* ADC #ab */
        data = dGetByte(PC++);
        goto adc;

    OPCODE(6a)              /* ROR */
        Z = N = (C << 7) | (A >> 1);
        C = A & 1;
        A = Z;
        break;

    OPCODE(6b)              /* ARR #ab [unofficial - Acc AND Data, ROR result] */
        /* It does some 'BCD fixup' if D flag is set */
        /* MPC 05/24/00 */
        data = A & dGetByte(PC++);
        if (regP & D_FLAG)
        {
            UWORD temp = (data >> 1) | (C << 7);
            Z = N = (UBYTE) temp;
            V = ((temp ^ data) & 0x40) >> 6;
            if (((data & 0x0F) + (data & 0x01)) > 5)
                temp = (temp & 0xF0) | ((temp + 0x6) & 0x0F);
            if (((data & 0xF0) + (data & 0x10)) > 0x50)
            {
                temp = (temp & 0x0F) | ((temp + 0x60) & 0xF0);
                C = 1;
            }
            else
                C = 0;
            A = (UBYTE) temp;
        }
        else
        {
            Z = N = A = (data >> 1) | (C << 7);
            C = (A & 0x40) >> 6;
            V = ((A >> 6) ^ (A >> 5)) & 1;
        }
        break;

    OPCODE(6c)              /* JMP (abcd) */
        addr = dGetWord(PC);
        if ((UBYTE) addr == 0xff)
            PC = (dGetByte(addr & ~0xff) << 8) | dGetByte(addr);
        else
            PC = dGetWord(addr);
        break;

    OPCODE(6d)              /* ADC abcd */
        ABSOLUTE;
        data = GetByte(addr);
        goto adc;

    OPCODE(6e)              /* ROR abcd */
        ABSOLUTE;
        RMW_GetByte(data, addr);
        Z = N = (C << 7) | (data >> 1);
        C = data & 1;
        PutByte(addr, Z);
        break;

    OPCODE(6f)              /* RRA abcd [unofficial - ROR Mem, then ADC to Acc] */
        ABSOLUTE;
        goto rra;

    OPCODE(70)              /* BVS */
        BRANCH(V)

    OPCODE(71)              /* ADC (ab),y */
        INDIRECT_Y;
        NCYCLES_Y;
        data = GetByte(addr);
        goto adc;

    OPCODE(73)              /* RRA (ab),y [unofficial - ROR Mem, then ADC to Acc] */
        INDIRECT_Y;
        goto rra;

    OPCODE(75)              /* ADC ab,x */
        ZPAGE_X;
        data = dGetByte(addr);
        goto adc;

    OPCODE(76)              /* ROR ab,x */
        ZPAGE_X;
        data = dGetByte(addr);
        Z = N = (C << 7) | (data >> 1);
        C = data & 1;
        dPutByte(addr, Z);
        break;

    OPCODE(77)              /* RRA zpage,x [unofficial - ROR Mem, then ADC to Acc] */
        ZPAGE_X;
        goto rra_zpage;

    OPCODE(78)              /* SEI */
        SetI;
        break;

    OPCODE(79)              /* ADC abcd,y */
        ABSOLUTE_Y;
        NCYCLES_Y;
        data = GetByte(addr);
        goto adc;

    OPCODE(7b)              /* RRA abcd,y [unofficial - ROR Mem, then ADC to Acc] */
        ABSOLUTE_Y;
        goto rra;

    OPCODE(7d)              /* ADC abcd,x */
        ABSOLUTE_X;
        NCYCLES_X;
        data = GetByte(addr);
        goto adc;

    OPCODE(7e)              /* ROR abcd,x */
        ABSOLUTE_X;
        RMW_GetByte(data, addr);
        Z = N = (C << 7) | (data >> 1);
        C = data & 1;
        PutByte(addr, Z);
        break;

    OPCODE(7f)              /* RRA abcd,x [unofficial - ROR Mem, then ADC to Acc] */
        ABSOLUTE_X;
        goto rra;

    OPCODE(81)              /* STA (ab,x) */
        INDIRECT_X;
        PutByte(addr, A);
        break;

    /* AXS doesn't change flags and SAX is better name for it (Fox) */
    OPCODE(83)              /* SAX (ab,x) [unofficial - Store result A AND X */
        INDIRECT_X;
        data = A & X;
        PutByte(addr, data);
        break;

    OPCODE(84)              /* STY ab */
        ZPAGE;
        dPutByte(addr, Y);
        break;

    OPCODE(85)              /* STA ab */
        ZPAGE;
        dPutByte(addr, A);
        break;

    OPCODE(86)              /* STX ab */
        ZPAGE;
        dPutByte(addr, X);
        break;

    OPCODE(87)              /* SAX zpage [unofficial - Store result A AND X] */
        ZPAGE;
        data = A & X;
        dPutByte(addr, data);
        break;

    OPCODE(88)              /* DEY */
        Z = N = --Y;
        break;

    OPCODE(8a)              /* TXA */
        Z = N = A = X;
        break;

    OPCODE(8b)              /* ANE #ab [unofficial - A AND X AND (Mem OR $EF) to Acc] (Fox) */
        data = dGetByte(PC++);
        N = Z = A & X & data;
        A &= X & (data | 0xef);
        break;

    OPCODE(8c)              /* STY abcd */
        ABSOLUTE;
        PutByte(addr, Y);
        break;

    OPCODE(8d)              /* STA abcd */
        ABSOLUTE;
        PutByte(addr, A);
        break;

    OPCODE(8e)              /* STX abcd */
        ABSOLUTE;
        PutByte(addr, X);
        break;

    OPCODE(8f)              /* SAX abcd [unofficial - Store result A AND X] */
        ABSOLUTE;
        data = A & X;
        PutByte(addr, data);
        break;

    OPCODE(90)              /* BCC */
        BRANCH(!C)

    OPCODE(91)              /* STA (ab),y */
        INDIRECT_Y;
        PutByte(addr, A);
        break;

    OPCODE(93)              /* SHA (ab),y [unofficial, UNSTABLE - Store A AND X AND (H+1) ?] (Fox) */
        /* It seems previous memory value is important - also in 9f */
        addr = dGetByte(PC++);
        data = dGetByte((UBYTE)(addr + 1)); /* Get high byte from zpage */
        data = A & X & (data + 1);
        addr = dGetWord(addr) + Y;
        PutByte(addr, data);
        break;

    OPCODE(94)              /* STY ab,x */
        ZPAGE_X;
        dPutByte(addr, Y);
        break;

    OPCODE(95)              /* STA ab,x */
        ZPAGE_X;
        dPutByte(addr, A);
        break;

    OPCODE(96)              /* STX ab,y */
        ZPAGE_Y;
        PutByte(addr, X);
        break;

    OPCODE(97)              /* SAX zpage,y [unofficial - Store result A AND X] */
        ZPAGE_Y;
        data = A & X;
        dPutByte(addr, data);
        break;

    OPCODE(98)              /* TYA */
        Z = N = A = Y;
        break;

    OPCODE(99)              /* STA abcd,y */
        ABSOLUTE_Y;
        PutByte(addr, A);
        break;

    OPCODE(9a)              /* TXS */
        S = X;
        break;

    OPCODE(9b)              /* SHS abcd,y [unofficial, UNSTABLE] (Fox) */
        /* Transfer A AND X to S, then store S AND (H+1)] */
        /* S seems to be stable, only memory values vary */
        addr = dGetWord(PC);
        PC += 2;
        S = A & X;
        data = S & ((addr >> 8) + 1);
        addr += Y;
        PutByte(addr, data);
        break;

    OPCODE(9c)              /* SHY abcd,x [unofficial - Store Y and (H+1)] (Fox) */
        /* Seems to be stable */
        addr = dGetWord(PC);
        PC += 2;
        /* MPC 05/24/00 */
        data = Y & ((UBYTE) ((addr >> 8) + 1));
        addr += X;
        PutByte(addr, data);
        break;

    OPCODE(9d)              /* STA abcd,x */
        ABSOLUTE_X;
        PutByte(addr, A);
        break;

    OPCODE(9e)              /* SHX abcd,y [unofficial - Store X and (H+1)] (Fox) */
        /* Seems to be stable */
        addr = dGetWord(PC);
        PC += 2;
        /* MPC 05/24/00 */
        data = X & ((UBYTE) ((addr >> 8) + 1));
        addr += Y;
        PutByte(addr, data);
        break;

    OPCODE(9f)              /* SHA abcd,y [unofficial, UNSTABLE - Store A AND X AND (H+1) ?] (Fox) */
        addr = dGetWord(PC);
        PC += 2;
        data = A & X & ((addr >> 8) + 1);
        addr += Y;
        PutByte(addr, data);
        break;

    OPCODE(a0)              /* LDY #ab */
        LDY(dGetByte(PC++));
        break;

    OPCODE(a1)              /* LDA (ab,x) */
        INDIRECT_X;
        LDA(GetByte(addr));
        break;

    OPCODE(a2)              /* LDX #ab */
        LDX(dGetByte(PC++));
        break;

    OPCODE(a3)              /* LAX (ind,x) [unofficial] */
        INDIRECT_X;
        Z = N = X = A = GetByte(addr);
        break;

    OPCODE(a4)              /* LDY ab */
        ZPAGE;
        LDY(dGetByte(addr));
        break;

    OPCODE(a5)              /* LDA ab */
        ZPAGE;
        LDA(dGetByte(addr));
        break;

    OPCODE(a6)              /* LDX ab */
        ZPAGE;
        LDX(dGetByte(addr));
        break;

    OPCODE(a7)              /* LAX zpage [unofficial] */
        ZPAGE;
        Z = N = X = A = GetByte(addr);
        break;

    OPCODE(a8)              /* TAY */
        Z = N = Y = A;
        break;

    OPCODE(a9)              /* LDA #ab */
        LDA(dGetByte(PC++));
        break;

    OPCODE(aa)              /* TAX */
        Z = N = X = A;
        break;

    OPCODE(ab)              /* ANX #ab [unofficial - AND #ab, then TAX] */
        Z = N = X = A &= dGetByte(PC++);
        break;

    OPCODE(ac)              /* LDY abcd */
        ABSOLUTE;
        LDY(GetByte(addr));
        break;

    OPCODE(ad)              /* LDA abcd */
        ABSOLUTE;
        LDA(GetByte(addr));
        break;

    OPCODE(ae)              /* LDX abcd */
        ABSOLUTE;
        LDX(GetByte(addr));
        break;

    OPCODE(af)              /* LAX absolute [unofficial] */
        ABSOLUTE;
        Z = N = X = A = GetByte(addr);
        break;

    OPCODE(b0)              /* BCS */
        BRANCH(C)

    OPCODE(b1)              /* LDA (ab),y */
        INDIRECT_Y;
        NCYCLES_Y;
        LDA(GetByte(addr));
        break;

    OPCODE(b3)              /* LAX (ind),y [unofficial] */
        INDIRECT_Y;
        Z = N = X = A = GetByte(addr);
        break;

    OPCODE(b4)              /* LDY ab,x */
        ZPAGE_X;
        LDY(dGetByte(addr));
        break;

    OPCODE(b5)              /* LDA ab,x */
        ZPAGE_X;
        LDA(dGetByte(addr));
        break;

    OPCODE(b6)              /* LDX ab,y */
        ZPAGE_Y;
        LDX(GetByte(addr));
        break;

    OPCODE(b7)              /* LAX zpage,y [unofficial] */
        ZPAGE_Y;
        Z = N = X = A = GetByte(addr);
        break;

    OPCODE(b8)              /* CLV */
        V = 0;
        break;

    OPCODE(b9)              /* LDA abcd,y */
        ABSOLUTE_Y;
        NCYCLES_Y;
        LDA(GetByte(addr));
        break;

    OPCODE(ba)              /* TSX */
        Z = N = X = S;
        break;

/* AXA [unofficial - original decode by R.Sterba and R.Petruzela 15.1.1998 :-)]
   AXA - this is our new imaginative name for instruction with opcode hex BB.
   AXA - Store Mem AND #$FD to Acc and X, then set stackpoint to value (Acc - 4)
   It's cool! :-)
   LAS - this is better name for this :) (Fox)
   It simply ANDs stack pointer with Mem, then transfers result to A and X
 */

    OPCODE(bb)              /* LAS abcd,y [unofficial - AND S with Mem, transfer to A and X (Fox) */
        ABSOLUTE_Y;
        Z = N = A = X = S &= GetByte(addr);
        break;

    OPCODE(bc)              /* LDY abcd,x */
        ABSOLUTE_X;
        NCYCLES_X;
        LDY(GetByte(addr));
        break;

    OPCODE(bd)              /* LDA abcd,x */
        ABSOLUTE_X;
        NCYCLES_X;
        LDA(GetByte(addr));
        break;

    OPCODE(be)              /* LDX abcd,y */
        ABSOLUTE_Y;
        NCYCLES_Y;
        LDX(GetByte(addr));
        break;

    OPCODE(bf)              /* LAX absolute,y [unofficial] */
        ABSOLUTE_Y;
        Z = N = X = A = GetByte(addr);
        break;

    OPCODE(c0)              /* CPY #ab */
        CPY(dGetByte(PC++));
        break;

    OPCODE(c1)              /* CMP (ab,x) */
        INDIRECT_X;
        CMP(GetByte(addr));
        break;

    OPCODE(c3)              /* DCM (ab,x) [unofficial - DEC Mem then CMP with Acc] */
        INDIRECT_X;

    dcm:
        RMW_GetByte(data, addr);
        data--;
        PutByte(addr, data);
        CMP(data);
        break;

    OPCODE(c4)              /* CPY ab */
        ZPAGE;
        CPY(dGetByte(addr));
        break;

    OPCODE(c5)              /* CMP ab */
        ZPAGE;
        CMP(dGetByte(addr));
        break;

    OPCODE(c6)              /* DEC ab */
        ZPAGE;
        Z = N = dGetByte(addr) - 1;
        dPutByte(addr, Z);
        break;

    OPCODE(c7)              /* DCM zpage [unofficial - DEC Mem then CMP with Acc] */
        ZPAGE;

    dcm_zpage:
        data = dGetByte(addr) - 1;
        dPutByte(addr, data);
        CMP(data);
        break;

    OPCODE(c8)              /* INY */
        Z = N = ++Y;
        break;

    OPCODE(c9)              /* CMP #ab */
        CMP(dGetByte(PC++));
        break;

    OPCODE(ca)              /* DEX */
        Z = N = --X;
        break;

    OPCODE(cb)              /* SBX #ab [unofficial - store (A AND X - Mem) in X] (Fox) */
        X &= A;
        data = dGetByte(PC++);
        C = X >= data;
        /* MPC 05/24/00 */
        Z = N = X -= data;
        break;

    OPCODE(cc)              /* CPY abcd */
        ABSOLUTE;
        CPY(GetByte(addr));
        break;

    OPCODE(cd)              /* CMP abcd */
        ABSOLUTE;
        CMP(GetByte(addr));
        break;

    OPCODE(ce)              /* DEC abcd */
        ABSOLUTE;
        RMW_GetByte(Z, addr);
        N = --Z;
        PutByte(addr, Z);
        break;

    OPCODE(cf)              /* DCM abcd [unofficial - DEC Mem then CMP with Acc] */
        ABSOLUTE;
        goto dcm;

    OPCODE(d0)              /* BNE */
        BRANCH(Z)

    OPCODE(d1)              /* CMP (ab),y */
        INDIRECT_Y;
        NCYCLES_Y;
        CMP(GetByte(addr));
        break;

    OPCODE(d3)              /* DCM (ab),y [unofficial - DEC Mem then CMP with Acc] */
        INDIRECT_Y;
        goto dcm;

    OPCODE(d5)              /* CMP ab,x */
        ZPAGE_X;
        CMP(dGetByte(addr));
        Z = N = A - data;
        C = (A >= data);
        break;

    OPCODE(d6)              /* DEC ab,x */
        ZPAGE_X;
        Z = N = dGetByte(addr) - 1;
        dPutByte(addr, Z);
        break;

    OPCODE(d7)              /* DCM zpage,x [unofficial - DEC Mem then CMP with Acc] */
        ZPAGE_X;
        goto dcm_zpage;

    OPCODE(d8)              /* CLD */
        ClrD;
        break;

    OPCODE(d9)              /* CMP abcd,y */
        ABSOLUTE_Y;
        NCYCLES_Y;
        CMP(GetByte(addr));
        break;

    OPCODE(db)              /* DCM abcd,y [unofficial - DEC Mem then CMP with Acc] */
        ABSOLUTE_Y;
        goto dcm;

    OPCODE(dd)              /* CMP abcd,x */
        ABSOLUTE_X;
        NCYCLES_X;
        CMP(GetByte(addr));
        break;

    OPCODE(de)              /* DEC abcd,x */
        ABSOLUTE_X;
        RMW_GetByte(Z, addr);
        N = --Z;
        PutByte(addr, Z);
        break;

    OPCODE(df)              /* DCM abcd,x [unofficial - DEC Mem then CMP with Acc] */
        ABSOLUTE_X;
        goto dcm;

    OPCODE(e0)              /* CPX #ab */
        CPX(dGetByte(PC++));
        break;

    OPCODE(e1)              /* SBC (ab,x) */
        INDIRECT_X;
        data = GetByte(addr);
        goto sbc;

    OPCODE(e3)              /* INS (ab,x) [unofficial - INC Mem then SBC with Acc] */
        INDIRECT_X;

    ins:
        RMW_GetByte(data, addr);
        N = Z = ++data;
        PutByte(addr, data);
        goto sbc;

    OPCODE(e4)              /* CPX ab */
        ZPAGE;
        CPX(dGetByte(addr));
        break;

    OPCODE(e5)              /* SBC ab */
        ZPAGE;
        data = dGetByte(addr);
        goto sbc;

    OPCODE(e6)              /* INC ab */
        ZPAGE;
        Z = N = dGetByte(addr) + 1;
        dPutByte(addr, Z);
        break;

    OPCODE(e7)              /* INS zpage [unofficial - INC Mem then SBC with Acc] */
        ZPAGE;

    ins_zpage:
        data = Z = N = dGetByte(addr) + 1;
        dPutByte(addr, data);
        goto sbc;

    OPCODE(e8)              /* INX */
        Z = N = ++X;
        break;

    OPCODE(e9)              /* SBC #ab */
    OPCODE(eb)              /* SBC #ab [unofficial] */
        data = dGetByte(PC++);
        goto sbc;

    OPCODE(ea)              /* NOP */
    OPCODE(1a)              /* NOP [unofficial] */
    OPCODE(3a)
    OPCODE(5a)
    OPCODE(7a)
    OPCODE(da)
    OPCODE(fa)
        break;

    OPCODE(ec)              /* CPX abcd */
        ABSOLUTE;
        CPX(GetByte(addr));
        break;

    OPCODE(ed)              /* SBC abcd */
        ABSOLUTE;
        data = GetByte(addr);
        goto sbc;

    OPCODE(ee)              /* INC abcd */
        ABSOLUTE;
        RMW_GetByte(Z, addr);
        N = ++Z;
        PutByte(addr, Z);
        break;

    OPCODE(ef)              /* INS abcd [unofficial - INC Mem then SBC with Acc] */
        ABSOLUTE;
        goto ins;

    OPCODE(f0)              /* BEQ */
        BRANCH(!Z)

    OPCODE(f1)              /* SBC (ab),y */
        INDIRECT_Y;
        NCYCLES_Y;
        data = GetByte(addr);
        goto sbc;

    OPCODE(f3)              /* INS (ab),y [unofficial - INC Mem then SBC with Acc] */
        INDIRECT_Y;
        goto ins;

    OPCODE(f5)              /* SBC ab,x */
        ZPAGE_X;
        data = dGetByte(addr);
        goto sbc;

    OPCODE(f6)              /* INC ab,x */
        ZPAGE_X;
        Z = N = dGetByte(addr) + 1;
        dPutByte(addr, Z);
        break;

    OPCODE(f7)              /* INS zpage,x [unofficial - INC Mem then SBC with Acc] */
        ZPAGE_X;
        goto ins_zpage;

    OPCODE(f8)              /* SED */
        SetD;
        break;

    OPCODE(f9)              /* SBC abcd,y */
        ABSOLUTE_Y;
        NCYCLES_Y;
        data = GetByte(addr);
        goto sbc;

    OPCODE(fb)              /* INS abcd,y [unofficial - INC Mem then SBC with Acc] */
        ABSOLUTE_Y;
        goto ins;

    OPCODE(fd)              /* SBC abcd,x */
        ABSOLUTE_X;
        NCYCLES_X;
        data = GetByte(addr);
        goto sbc;

    OPCODE(fe)              /* INC abcd,x */
        ABSOLUTE_X;
        RMW_GetByte(Z, addr);
        N = ++Z;
        PutByte(addr, Z);
        break;

    OPCODE(ff)              /* INS abcd,x [unofficial - INC Mem then SBC with Acc] */
        ABSOLUTE_X;
        goto ins;

    OPCODE(d2)              /* ESCRTS #ab (CIM) - on Atari is here instruction CIM [unofficial] !RS! */

        SA_C6502_RETURN;
        break;

    OPCODE(f2)              /* ESC #ab (CIM) - on Atari is here instruction CIM [unofficial] !RS! */

        SA_C6502_RETURN;
        break;

    OPCODE(02)              /* CIM [unofficial - crash intermediate] */
    OPCODE(12)
    OPCODE(22)
    OPCODE(32)
    OPCODE(42)
    OPCODE(52)
    OPCODE(62)
    OPCODE(72)
    OPCODE(92)
    OPCODE(b2)

        SA_C6502_RETURN;
        break;

/* ---------------------------------------------- */
/* ADC and SBC routines */

    adc:
        if (!(regP & D_FLAG)) {
            UWORD tmp;      /* Binary mode */
            tmp = A + data + (UWORD)C;
            C = tmp > 0xff;
            V = !((A ^ data) & 0x80) && ((A ^ tmp) & 0x80);
            Z = N = A = (UBYTE) tmp;
        }
        else {
            UWORD tmp;      /* Decimal mode */
            tmp = (A & 0x0f) + (data & 0x0f) + (UWORD)C;
            if (tmp >= 10)
                tmp = (tmp - 10) | 0x10;
            tmp += (A & 0xf0) + (data & 0xf0);

            Z = A + data + (UWORD)C;
            N = (UBYTE) tmp;
            V = !((A ^ data) & 0x80) && ((A ^ tmp) & 0x80);

            if (tmp > 0x9f)
                tmp += 0x60;
            C = tmp > 0xff;
            A = (UBYTE) tmp;
        }
        break;

    sbc:
        if (!(regP & D_FLAG)) {
            UWORD tmp;      /* Binary mode */
            tmp = A - data - !C;
            C = tmp < 0x100;
            V = ((A ^ tmp) & 0x80) && ((A ^ data) & 0x80);
            Z = N = A = (UBYTE) tmp;
        }
        else {
            UWORD al, ah, tmp;  /* Decimal mode */
            tmp = A - data - !C;
            al = (A & 0x0f) - (data & 0x0f) - !C;   /* Calculate lower nybble */
            ah = (A >> 4) - (data >> 4);        /* Calculate upper nybble */
            if (al & 0x10) {
                al -= 6;    /* BCD fixup for lower nybble */
                ah--;
            }
            if (ah & 0x10) ah -= 6;     /* BCD fixup for upper nybble */

            C = tmp < 0x100;            /* Set flags */
            V = ((A ^ tmp) & 0x80) && ((A ^ data) & 0x80);
            Z = N = (UBYTE) tmp;

            A = (ah << 4) | (al & 0x0f);    /* Compose result */
        }
        break;

    }

        continue;
    }

    SA_C6502_RETURN;
}

void CPU_Initialise(void)
{
}
