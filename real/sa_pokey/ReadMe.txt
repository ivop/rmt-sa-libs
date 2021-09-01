========================================================================
DYNAMIC LINK LIBRARY: sa_pokey.dll                          license: GPL
Standalone Pokey emulation library, by C.P.U. 2003
========================================================================

This is the library created from the original Ron Fries POKEY (Atari
custom chip) emulator. Code was taken from Atari800 emulator 1.2.4
(http://atari800.sourceforge.net) and was stripped down for sound
generation only and lacks a lot of the original functionality (IRQ,
timers, keyboard, random number, potentiometers, etc.)

It provides this functions:

void Pokey_Initialise(int *argc, char *argv[]);
void Pokey_SoundInit(uint32 freq17, uint16 playback_freq, uint8 num_pokeys);
void Pokey_Process(uint8 * sndbuffer, const uint16 sndn);
UBYTE Pokey_GetByte(UWORD addr);
void Pokey_PutByte(UWORD addr, UBYTE byte);
void Pokey_About(char** name, char** author, char** description );

Note:

Full source codes are in the "src" directory.

Known bugs:

This version contains some bugs, therefore there catch of sounds appears 
sometimes or a part of sound data fails and POKEY emulation is inaccurate.
We are sorry, we weren't be able to find out this bugs so far.
