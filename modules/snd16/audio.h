/* audio.h - interactions with SDL for CHIP16 sound device
   Authors: Grigory Rechistov, 2014, and contributors of the MIPT Ilab project.
 */
#ifndef CHIP16_AUDIO_H
#define CHIP16_AUDIO_H

#include <stdint.h>

typedef enum {
    audio_meandre = 0,
    audio_triangle,
    audio_sawtooth,
    audio_pulse,
    audio_noise
} wave_type_t;

typedef struct audio_params {
    wave_type_t wave_type;
    uint64_t param; //TODO expand to more state.
} audio_params_t;

void waveform_callback(void* userdata, Uint8* stream, int len);
   

#endif // CHIP16_AUDIO_H
