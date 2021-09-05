#if _MSC_VER > 1000
#pragma once
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    return TRUE;
}


void __declspec(dllexport) C6502_About(char** name, char** author, char** description )
{
    *name =        (char*)"CPU 6502 emulation, 2021.09";
    *author =      (char*)"Atari800 emulator developer team, https://atari800.github.io/";
    *description = (char*)"Stripped down C.P.U. 2003, cleaned up and code injection by ivop 2021";
}
