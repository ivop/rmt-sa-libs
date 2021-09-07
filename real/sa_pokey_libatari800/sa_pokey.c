#include <stdio.h>
#include <stdint.h>
#include "libatari800.h"

static char *my_argv[] = { "-config", "atari800.cfg", NULL };

void __declspec(dllexport) Pokey_Initialise(int *argc, char *argv[]) {
    fprintf(stderr, "%s: argc=%i\n", __func__, argc);

    libatari800_init(-1, my_argv);
}

void __declspec(dllexport) Pokey_SoundInit(uint32_t freq17,
                                           uint16_t playback_freq,
                                           uint8_t num_pokeys) {
    fprintf(stderr, "%s: freq17=%u playback_freq=%u num_pokeys=%u\n",
            __func__, freq17, playback_freq, num_pokeys);
}

void __declspec(dllexport) Pokey_Process(uint8_t *sndbuffer,
                                         const uint16_t sndn) {
//    fprintf(stderr, "%s: sndn=%u\n", __func__, sndn);
}

uint8_t __declspec(dllexport) Pokey_GetByte(uint16_t addr) {
    fprintf(stderr, "%s: addr=%u\n", __func__, addr);
    return 0;
}

void __declspec(dllexport) Pokey_PutByte(uint16_t addr, uint8_t byte) {
//    fprintf(stderr, "%s: addr=%u byte=%u\n", __func__, addr, byte);
}

void __declspec(dllexport) Pokey_About(char** name,
                                       char** author,
                                       char** description ) {
    *name        = (char*) "Bleh bleh bleh";
    *author      = (char*) "Dracula";
    *description = (char*) "I never say that!";
}
