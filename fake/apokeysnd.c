#include <stdio.h>

void __declspec(dllexport) APokeySound_Initialize(int stereo) {
    fprintf(stderr, "%s: stereo=%i\n", __func__, stereo);
}

void __declspec(dllexport) APokeySound_PutByte(int addr, int data) {
//    fprintf(stderr, "%s: addr=%i data=%i\n", __func__, addr, data);
}

int __declspec(dllexport) APokeySound_GetRandom(int addr, int cycle) {
    fprintf(stderr, "%s: addr=%i cycle=%i\n", __func__, addr, cycle);
    return 0;
}

int __declspec(dllexport) APokeySound_Generate(int cycles,
                                               unsigned char *buffer,
                                               int depth) {
//    fprintf(stderr, "%s: cycles=%i depth=%i\n", __func__, cycles, depth);
	return cycles;
}

void __declspec(dllexport) APokeySound_About(const char **name,
                                             const char **author,
                                             const char **description) {
	*name = "apokeysnd dummy";
	*author = "me";
	*description = "dummy";
}

