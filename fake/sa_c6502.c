#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

int prev = 0;

void __declspec(dllexport) C6502_Initialise(uint8_t *memory ) {
    fprintf(stderr, "%s: memory pointer set\n", __func__);
}

int __declspec(dllexport) C6502_JSR(uint16_t *adr,
                                    uint8_t *areg,
                                    uint8_t *xreg,
                                    uint8_t *yreg, int *maxcycles) {

    struct timeval  tv;
    gettimeofday(&tv, NULL);

    fprintf(stderr, "%s: PC=%04x A=%02x X=%02x Y=%02x maxcycles=%i\n",
            __func__, *adr, *areg, *xreg, *yreg, *maxcycles);

    fprintf(stderr, "%s: ms=%u\n", __func__, (tv.tv_usec - prev)/1000);

    prev = tv.tv_usec;
    return (*maxcycles)/2;
}

void __declspec(dllexport) C6502_About(char** name,
                                       char** author,
                                       char** description ) {
    *name        = "Dummy CPU";
    *author      = "Dummy";
    *description = "Doing nothing";
}

