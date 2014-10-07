/* audio.c - interactions with SDL for CHIP16 sound device
   Authors: Grigory Rechistov, 2014, and contributors of the MIPT Ilab project.
 */

#include "include/SDL2/SDL.h"

const int audio_volume = 1000;
const int audio_frequency = 500;

static void meandre_callback(void* userdata, Uint8* stream, int len) {
    // len /= 2; /* 16 bit */
    Sint16* buf = (Sint16*)stream;
    Sint16 sign = 1;
    for(int i = 0; i < len; i++) { 
        buf[i] = sign * audio_volume * audio_position * audio_frequency;
        audio_position++;
        sign = -sign; // TODO: waveshape is wrong.
    }
    // audio_len -= len;
}


void waveform_callback(void* userdata, Uint8* stream, int len) {
    const audio_params_t *ap = (audio_params_t*)userdata;
    switch (ap->wave_type) {
        case audio_meandre:
            meandre_callback(userdata, stream, len);
            break;
        default:
            assert(0);
    }
}
