#include "stdafx.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


void __declspec(dllexport) C6502_About(char** name, char** author, char** description )
{
	*name =        (char*)"CPU 6502 emulation, 1.2.4 - cross compiled at 2021-08-31";
	*author =      (char*)"Atari800 emulator developer team, http://atari800.sourceforge.net";
	*description = (char*)"Stripped down version for instructions execution only by C.P.U. 2003";
}
