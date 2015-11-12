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
#include <limits.h>
#include <simics/util/help-macros.h>
#include <simics/simulator-api.h>

#include "include/SDL2/SDL.h"
#include "audio.h"

#define SND_DEBUG 1 // Dump waveform

// Fill stream with silence
// IN:  userdata        - converted to audio_params, current phase and parameters
// IN:  len_raw         - length in bytes, NOT samples!
// IN:  advance_time    - whether to change phase or not
// OUT: userdata        - phase and waveform-specfic state updated
// OUT: stream          - samples for waveform
/*
static void silence (void* userdata, uint8_t* stream, int len_raw, bool advance_time)
        {
        int len = len_raw / 2; // 16 bit

        // silence is just zeroes in all formats
        assert (userdata);
        audio_params_t *ap = (audio_params_t*)userdata;

        memset (stream, 0, len_raw);
        if (advance_time)
                ap->phase += len;
        };
*/

// fill stream with meandre (rectangular waveform)
// IN:  userdata        - converted to audio_params, current phase and parameters
// IN:  len_raw         - length in bytes, NOT samples!
// OUT: userdata        - phase and waveform-specfic state updated
// OUT: stream          - samples for waveform
static void meandre (void* userdata, uint8_t* stream, int len_raw) {
        int len = len_raw / 2; /* 16 bit */

        assert (userdata);
        audio_params_t *ap = (audio_params_t*)userdata;

        //assert (ap->limit > ap->phase);
        int16_t* buf = (int16_t*)stream;
      
        //int silent_samples = 0;
        
        int16_t sign = (ap->sign != 0)? ap->sign: 1;
        assert (sign == 1 || sign == -1);
        for (int i = 0; i < len; i++) {
                buf[i] = sign * ap->sdl_vol;
                // Signal sign changes if time for this sample and next sample times
                // are across a  boundary of period.
                // Detect boundary as a floor value of current time divied by period
                if ( (uint64_t)(ap->sample_len * (ap->phase+1)* ap->signal_freq ) !=
                     (uint64_t)(ap->sample_len *  ap->phase   * ap->signal_freq ))
                        sign *= -1;
                ap->phase++;
        }
        //ap->phase += silent_samples;      account for samples spent for silence

        // save flag for the next invocation
        ap->sign = sign;
}

// fill stream with noise
static void noise (void* userdata, uint8_t* stream, int len_raw) {
        int len = len_raw / 2; /* 16 bit */

        assert (userdata);
        audio_params_t *ap = (audio_params_t*)userdata;

        // assert (ap->limit > ap->phase);
        int16_t* buf = (int16_t*)stream;
        //int silent_samples = 0;

        // checking for future integers overflow
        CASSERT_STMT (RAND_MAX <= (LONG_MAX / 2));
        for (int i = 0; i < len; i++) {
                buf[i] = (int16_t)((long)ap->sdl_vol * (2 * rand() - RAND_MAX) / RAND_MAX);
                ap->phase++;
        }

        //ap->phase += silent_samples;        // account for samples spent for silence
}

static void sawtooth (void* userdata, uint8_t* stream, int len_raw) {
        int len = len_raw / 2; /* 16 bit */

        assert (userdata);
        audio_params_t* ap = (audio_params_t*)userdata;

        int16_t* buf = (int16_t*)stream;
        int64_t saw_phase = (int64_t)ap->phase;
        uint32_t slope = ap->sdl_vol * 2 * ap->signal_freq; //slope of the sawtooth k = 2A/T;

        for (int i = 0; i < len; i++) {
                buf[i] = (int16_t)(slope * ap->sample_len * saw_phase - ap->sdl_vol);

                if ( (int64_t)(slope * ap->sample_len * (saw_phase + 1) - ap->sdl_vol) > (int64_t)ap->sdl_vol )
                        saw_phase = 0 - 1;  //because phase should be 0, and next step is phase++
                saw_phase++;
        }
        ap->phase = (uint64_t)saw_phase; //save phase for the next invocation
}

static void triangle (void* userdata, uint8_t* stream, int len_raw) {
        int len = len_raw / 2;  /*16 bit*/ 
        assert (userdata);
        audio_params_t *ap = (audio_params_t*)userdata;
        //assert (ap->limit > ap->phase);

        int16_t * buf = (int16_t*)stream, \
                sign = (ap->sign != 0)? ap->sign: 1, \
                volume = ap->sdl_vol;
        double samples_per_slope = ap->period / 2 / ap->sample_len;
        double init_phase = ap->phase;
        init_phase -= (unsigned)(init_phase / samples_per_slope) * samples_per_slope;
        uint32_t slope = (volume * 2.0) * (ap->signal_freq / 2);

        double delta = slope * ap->sample_len;
        double cur_vol = sign * (ap->sample_len * init_phase * slope - volume);
        //SIM_printf("===delta = %g\n\tslope = %d, cur_vol = %lf\n", delta, slope, cur_vol);

        for (int i = 0; i < len; i++) {
                buf[i] = (int16_t)cur_vol;
                //SIM_printf("===== %lf =====\n", cur_vol);
                cur_vol += sign * delta;
                if ((cur_vol >= volume) || (cur_vol <= - volume)) {
                        cur_vol = sign * 2 * volume - cur_vol;
                        sign = -sign;
                }
                ap->phase++;
        }
        // save for the next invocation
        ap->sign = sign;
}

// Dump just generated sound buffer to textual representation for debug purposes.
// IN:  userdata        - for current phase and parameters
// IN:  stream          - buffer to be dumped
// IN:  len_raw         - length of buffer in ELEMENTS.
// IN:  out             - file to dump in (needed to be initialized and closed outside)

static void
dump_waveform(const audio_params_t * ap, const int16_t * stream, int len, FILE * out) {
        assert (ap);
       
        uint64_t cur_sample = ap->phase - len;
        double dt = ap->sample_len;

        SIM_printf ("# Dump for waveform with number %d\n\t\
starting from %16lu sample, time is %12g\n", ap->wave_type, cur_sample, dt * cur_sample);
        for (int i = 0; i < len; i++) {
                // time (double) value (int16_t)
                fprintf (out, "%8.8g %+6d\n", dt * cur_sample, stream[i]);
                cur_sample++;
        }
}

// Callback used by SDL when it needs a new portion of waveform.
// IN:  userdata        - converted to audio_params, current phase and signal parameters
// IN:  len_raw         - length in bytes, NOT samples!
// OUT: userdata        - phase and waveform-specfic state updated
// OUT: stream          - samples for waveform
void waveform_callback(void* userdata, uint8_t* stream, int len_raw) {
        assert (userdata);
        const audio_params_t *ap = (audio_params_t*)userdata;
        
        switch (ap->wave_type) {
        case Audio_Meandre:
                meandre (userdata, stream, len_raw);
                break;
        case Audio_Noise:
                noise (userdata, stream, len_raw);
                break;
        case Audio_Sawtooth:
                sawtooth (userdata, stream, len_raw);
                break;
        case Audio_Triangle:
                triangle (userdata, stream, len_raw);
                break;
        default:
                assert (0 && "unimplemented waveform");
        }
#ifdef SND_DEBUG
        FILE * out;
        out = fopen("logs/dump.txt", "a");
        
        dump_waveform ((audio_params_t *)userdata, (int16_t *)stream, len_raw / 2, out);
        fclose(out);
#endif
}

