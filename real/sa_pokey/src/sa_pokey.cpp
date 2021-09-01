// sa_pokey.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


void __declspec(dllexport) Pokey_About(char** name, char** author, char** description )
{
	*name =        (char*)"POKEY Chip Simulator, V2.3 (A800 1.2.4)";
	*author =      (char*)"Ron Fries, Atari800 emulator developer team, http://atari800.sourceforge.net";
	*description = (char*)"Stripped down version for Pokey sound generation only by C.P.U. 2003";
}