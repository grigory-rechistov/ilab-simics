/* audio.h - interactions with SDL for CHIP16 sound device
   Authors: Grigory Rechistov, 2014, and contributors of the MIPT Ilab project.
 */
#ifndef CHIP16_AUDIO_H
#define CHIP16_AUDIO_H

#include <stdint.h>

typedef enum {
    Audio_Meandre = 0,
    Audio_Triangle,
    Audio_Sawtooth,
    Audio_Pulse,
    Audio_Noise
} wave_type_t;

typedef struct audio_params {
    // host card parameters
    int freq;              // sampling frequency, Hz
    double sample;         // length of sample in seconds, == 1.0d / freq

    // common waveform parameters
    wave_type_t wave_type; // see above
    int16_t sdl_vol;       // volume from 0 to 32767
    uint32_t signal_freq;  // oscillation frequency, Hz
    double period;         // length of one oscillation in seconds, == 1.0d / signal_freq, unless zero, then it is undefined
    uint64_t phase;        // current sample
    uint64_t limit;        // maximum sample for this waveform
    
    // waveform-specific parameters
    int sign;
    
} audio_params_t;

void waveform_callback(void* userdata, uint8_t* stream, int len);
   

#endif // CHIP16_AUDIO_H
