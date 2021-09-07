#include <stdio.h>
#include <stdint.h>
#include "libatari800.h"

static char *my_argv[] = { "-config", "atari800.cfg", NULL };

static input_template_t my_input;

void __declspec(dllexport) Pokey_Initialise(int *argc, char *argv[]) {
    fprintf(stderr, "%s: argc=%i\n", __func__, argc);

    if (!libatari800_init(-1, my_argv)) {
        fprintf(stderr, "%s: libatari800_init failed\n", __func__);
        return;
    } else {
        fprintf(stderr, "%s: libatari800_init succeeded\n", __func__);
    }

    libatari800_clear_input_array(&my_input);
    if (!libatari800_reboot_with_file("mirror.xex")) {
        fprintf(stderr, "%s: libatari800_reboot_with_file failed\n", __func__);
        return;
    } else {
        fprintf(stderr, "%s: libatari800_reboot_with_file succeeded\n",
                                                                    __func__);
    }
 
    // run a few frames
    for (int i=0; i<4; i++) {
        int r = libatari800_next_frame(&my_input); // 0 = success
        if (!r) fprintf(stderr, "%s: libatari800_next_frame succeeded\n",
                                                                    __func__);
        else    fprintf(stderr, "%s: libatari800_next_frame failed\n",
                                                                    __func__);
    }

    fprintf(stderr, "%s: libatari800_get_sound_frequency = %i\n",
                                __func__, libatari800_get_sound_frequency());
    fprintf(stderr, "%s: libatari800_get_num_sound_channels = %i\n",
                               __func__, libatari800_get_num_sound_channels());
    fprintf(stderr, "%s: libatari800_get_sound_sample_size = %s\n",
            __func__, libatari800_get_sound_sample_size() == 1 ?
            "8-bit audio" : "16-bit audio");
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

