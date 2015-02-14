/* audio.c - interactions with SDL for CHIP16 sound device
 * Authors: Grigory Rechistov, 2014, and contributors of the MIPT Ilab project.
 */

/* NOTE: all of this code implicitly assumes that audio output format is
 * signed 16-bit integer. It might be beneficial to generalize this
 * to support arbitrary SDL-supported audio formats.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "include/SDL2/SDL.h"
#include "audio.h"

#define SND_DEBUG 1 // Dump waveform to console

// Fill stream with silence
// IN:  userdata        - converted to audio_params, current phase and parameters
// IN:  len_raw         - length in bytes, NOT samples!
// IN:  advance_time    - whether to change phase or not
// OUT: userdata        - phase and waveform-specfic state updated
// OUT: stream          - samples for waveform
static void silence(void* userdata, uint8_t* stream, int len_raw, bool advance_time) {
    int len = len_raw / 2 /* 16 bit */;
    // silence is just zeroes in all formats
    assert(userdata);
    audio_params_t *ap = (audio_params_t*)userdata;
    memset(stream, 0, len_raw);
    if (advance_time)
        ap->phase += len;
};

// Fill stream with meandre (rectangular waveform)
// IN:  userdata        - converted to audio_params, current phase and parameters
// IN:  len_raw         - length in bytes, NOT samples!
// OUT: userdata        - phase and waveform-specfic state updated
// OUT: stream          - samples for waveform
static void meandre(void* userdata, uint8_t* stream, int len_raw) {
    int len = len_raw / 2; /* 16 bit */
    assert(userdata);
    audio_params_t *ap = (audio_params_t*)userdata;
    assert(ap->limit > ap->phase);
    int16_t* buf = (int16_t*)stream;
    int newlen = ap->limit - ap->phase;
    int silent_samples = 0;
    if (len > newlen) {
        // sound should stop earlier than buffer
        // adjust len, and fill the end of the buffer with silence
        silent_samples = len - newlen;
        silence(userdata, stream + newlen*2, silent_samples*2, false);
        len = newlen;
    }
    int16_t sign = (ap->sign != 0)? ap->sign: 1;
    assert(sign == 1 || sign == -1);
    for (int i = 0; i < len; i++) {
        buf[i] = sign * ap->sdl_vol;
        // Signal sign changes if time for this sample and next sample times
        // are across a  boundary of period.
        // Detect boundary as a floor value of current time divied by period
        if ( (uint64_t)(ap->sample * (ap->phase+1)* ap->signal_freq ) !=
             (uint64_t)(ap->sample *  ap->phase   * ap->signal_freq ))
            sign = -sign;
        ap->phase++;
    }
    // account for samples spent for silence
    ap->phase += silent_samples;
    // save flag for the next invocation
    ap->sign = sign;
}


// Dump just generated sound buffer to textual representation for debug purposes.
// IN:  userdata        - current phase and parameters
// IN:  stream          - buffer to be dumped
// IN:  len_raw         - length of buffer in bytes.
static void dump_waveform(const void* userdata, const uint8_t* stream, int len_raw) {
    int len = len_raw / 2; /* 16 bit */
    assert(userdata);
    const audio_params_t *ap = (audio_params_t*)userdata;
    const int16_t* buf = (int16_t*)stream;

    uint64_t cursample = ap->phase - len;
    double delta = ap->sample;
    printf("# Starting from %16lu %6.6g\n", cursample, delta * cursample);
    for (int i = 0; i < len; i++) {
        // sample (int) time (double) value (int16_t)
        printf("%16lu %8.8g %+6d\n", cursample, delta * cursample, buf[i]);
        cursample++;
    }
}

// Callback used by SDL when it needs a new portion of waveform.
// IN:  userdata        - converted to audio_params, current phase and signal parameters
// IN:  len_raw         - length in bytes, NOT samples!
// OUT: userdata        - phase and waveform-specfic state updated
// OUT: stream          - samples for waveform
void waveform_callback(void* userdata, uint8_t* stream, int len_raw) {
    assert(userdata);
    const audio_params_t *ap = (audio_params_t*)userdata;

    if ((ap->signal_freq == 0) || // signal is not oscillating
         ap->limit <= ap->phase   // we are asked for sound while we've already given all we got
    ) {
        silence(userdata, stream, len_raw, true);
    } else {
        switch (ap->wave_type) {
            case Audio_Meandre:
                meandre(userdata, stream, len_raw);
                break;
            default:
                assert(0 && "unimplemented waveform");
        }
    }
    (void)(SND_DEBUG? dump_waveform(userdata, stream, len_raw): 0);
}
