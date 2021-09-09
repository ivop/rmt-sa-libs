#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "libatari800.h"

static char *argv_pal[]  = { "-config", "atari800-pal.cfg",  NULL };
static char *argv_ntsc[] = { "-config", "atari800-ntsc.cfg", NULL };

static char **my_argv;

static input_template_t my_input;
static uint8_t *memory;

static uint8_t *temp_buffer[65536];     // more than big enough :)
static int bytes_in_temp_buffer = 0;

#define MIRROR_POKEY 0x8200

// ----------------------------------------------------------------------------

void __declspec(dllexport) Pokey_Initialise(int *argc, char *argv[]) {
    fprintf(stderr, "%s: argc=%i\n", __func__, argc);
}

// ----------------------------------------------------------------------------

void __declspec(dllexport) Pokey_SoundInit(uint32_t freq17,
                                           uint16_t playback_freq,
                                           uint8_t num_pokeys) {
    fprintf(stderr, "%s: freq17=%u playback_freq=%u num_pokeys=%u\n",
            __func__, freq17, playback_freq, num_pokeys);

    if (freq17 == 1789790) {
        fprintf(stderr, "%s: NTSC detected\n\n", __func__);
        my_argv = argv_ntsc;
    } else if (freq17 == 1773447) {
        fprintf(stderr, "%s: PAL detected\n\n", __func__);
        my_argv = argv_pal;
    } else {
        fprintf(stderr, "%s: unknown frequency, default NTSC\n\n", __func__);
        my_argv = argv_ntsc;
    }

    if (!libatari800_init(-1, my_argv)) {
        fprintf(stderr, "%s: libatari800_init failed\n", __func__);
        return;
    } else {
        fprintf(stderr, "%s: libatari800_init succeeded\n", __func__);
    }

    libatari800_clear_input_array(&my_input);

    fprintf(stderr, "%s: reboot with mirror.xex\n", __func__);

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
//        fprintf(stderr, "%s: libatari800_get_sound_buffer_len = %i\n",
//                __func__, libatari800_get_sound_buffer_len());
    }

    fprintf(stderr, "%s: libatari800_get_sound_frequency = %i\n",
                                __func__, libatari800_get_sound_frequency());
    fprintf(stderr, "%s: libatari800_get_num_sound_channels = %i\n",
                               __func__, libatari800_get_num_sound_channels());
    fprintf(stderr, "%s: libatari800_get_sound_sample_size = %s\n",
            __func__, libatari800_get_sound_sample_size() == 1 ?
            "8-bit audio" : "16-bit audio");
    fprintf(stderr, "%s: libatari800_get_fps = %f\n",
                                __func__, libatari800_get_fps());

    memory = libatari800_get_main_memory_ptr();
    fprintf(stderr, "%s: copied memory pointer\n", __func__);
}

// ----------------------------------------------------------------------------

void __declspec(dllexport) Pokey_Process(uint8_t *sndbuffer,
                                         const uint16_t sndn) {
//    fprintf(stderr, "%s: %i bytes requested\n", __func__, sndn);
#ifdef WHITE_NOISE_TEST
    for (int i=0; i<sndn; i++)
        sndbuffer[i] = rand();
    return;
#endif

    while (bytes_in_temp_buffer < sndn) {
        int r = libatari800_next_frame(&my_input); // 0 = success
        if (r) {
            fprintf(stderr, "%s: next_frame failed\n", __func__);
            return;
        }

        uint8_t *libatari800_sound_buffer = libatari800_get_sound_buffer();
        int libatari800_sound_buffer_len = libatari800_get_sound_buffer_len();

        memcpy(temp_buffer + bytes_in_temp_buffer,
               libatari800_sound_buffer, libatari800_sound_buffer_len);

        bytes_in_temp_buffer += libatari800_sound_buffer_len;

//        fprintf(stderr, "%s: added %i bytes to temp_buffer\n", __func__,
//                libatari800_sound_buffer_len);
    }

    memcpy(sndbuffer, temp_buffer, sndn);
    memcpy(temp_buffer, temp_buffer+sndn, bytes_in_temp_buffer - sndn);

    bytes_in_temp_buffer -= sndn;
}

// ----------------------------------------------------------------------------

uint8_t __declspec(dllexport) Pokey_GetByte(uint16_t addr) {
    fprintf(stderr, "%s: addr=%u\n", __func__, addr);
    return 0;
}

// ----------------------------------------------------------------------------

// addr is relative to the start of the first pokey!

void __declspec(dllexport) Pokey_PutByte(uint16_t addr, uint8_t byte) {
//    fprintf(stderr, "%s: addr=%u byte=%u\n", __func__, addr, byte);

    memory[MIRROR_POKEY+addr] = byte;
}

// ----------------------------------------------------------------------------

void __declspec(dllexport) Pokey_About(char** name,
                                       char** author,
                                       char** description ) {
    *name        = (char*) "Bleh bleh bleh";
    *author      = (char*) "Dracula";
    *description = (char*) "I never say that!";
}

