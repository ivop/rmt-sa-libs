// sa_pokey.c : Defines the entry point for the DLL application.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

void __declspec(dllexport) Pokey_About(char** name, char** author, char** description )
{
	*name =        (char*)"POKEY Chip Simulator, V2.3 (A800 1.2.4) -- experimental ivop fork";
	*author =      (char*)"Ron Fries, Atari800 emulator developer team, http://atari800.sourceforge.net";
	*description = (char*)"Stripped down version for Pokey sound generation only by C.P.U. 2003";
}
