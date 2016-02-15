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


// fill stream with meandre (rectangular waveform)
// IN:  userdata        - converted to audio_params, current phase and parameters
// IN:  len_raw         - length in bytes, NOT samples!
// OUT: userdata        - phase and waveform-specfic state updated
// OUT: stream          - samples for waveform
static void meandre (void* userdata, uint8_t* stream, int len_raw) {
        int len = len_raw / 2; /* 16 bit */
        assert (userdata);
        audio_params_t *ap = (audio_params_t*)userdata;
        int16_t* buf = (int16_t*)stream;
        int16_t sign = (ap->sign != 0)? ap->sign: 1;
        assert (sign == 1 || sign == -1);

        for (int i = 0; i < len; i++) {
                buf[i] = sign * ap->signal_vol;
                // Signal sign changes if time for this sample and next sample times
                // are across a  boundary of period.
                // Detect boundary as a floor value of current time divied by period
                if ( (uint64_t)(ap->sample_len * (ap->phase+1)* ap->freq ) !=
                     (uint64_t)(ap->sample_len *  ap->phase   * ap->freq ))
                        sign *= -1;
                ap->phase++;
        }
        // save flag for the next invocation
        ap->sign = sign;
}

// fill stream with noise
static void noise (void* userdata, uint8_t* stream, int len_raw) {
        int len = len_raw / 2; /* 16 bit */
        assert (userdata);
        audio_params_t *ap = (audio_params_t*)userdata;
        int16_t* buf = (int16_t*)stream;

        // checking for future integers overflow
        CASSERT_STMT (RAND_MAX <= (LONG_MAX / 2));
        for (int i = 0; i < len; i++) {
                buf[i] = (int16_t)((long)ap->signal_vol * (2 * rand() - RAND_MAX) / RAND_MAX);
                ap->phase++;
        }
}

static void sawtooth (void* userdata, uint8_t* stream, int len_raw) {
        int len = len_raw / 2; /* 16 bit */
        assert (userdata);
        audio_params_t* ap = (audio_params_t*)userdata;
        int16_t* buf = (int16_t*)stream;

        uint32_t slope = (uint32_t) (ap->signal_vol * 2 * ap->freq);    //slope of the sawtooth k = 2A/T;
        double delta = slope * ap->sample_len;
        double samples_per_period = (double)(ap->period / ap->sample_len);  //double number of samples in one period
        double init_phase = ap->phase;                                      //phase we need to calculate cur_vol
        init_phase -= (unsigned)(init_phase / samples_per_period) * samples_per_period;
        double cur_vol = ap->sample_len * init_phase * slope - ap->signal_vol;

        for (int i = 0; i < len; i++) {
                buf[i] = (int16_t)cur_vol;
                cur_vol += delta;

                if ( (uint64_t)(ap->sample_len * (ap->phase+1)* ap->freq ) !=
                     (uint64_t)(ap->sample_len *  ap->phase   * ap->freq ))
                        cur_vol -= 2 * ap->signal_vol;
                ap->phase++;
        }

}

static void triangle (void* userdata, uint8_t* stream, int len_raw) {
        int len = len_raw / 2;  /*16 bit*/
        assert (userdata);
        audio_params_t *ap = (audio_params_t*)userdata;
        int16_t * buf = (int16_t*)stream;
        int16_t sign = (ap->sign != 0)? ap->sign: 1;
        int16_t volume = ap->signal_vol;
        double samples_per_slope = ap->period / 2 / ap->sample_len;//samples per half of triangle
        double init_phase = ap->phase;
        init_phase -= (unsigned)(init_phase / samples_per_slope) * samples_per_slope;
        uint32_t slope = (volume * 2.0) * (ap->freq * 2); //slope of half of triangle

        double delta = slope * ap->sample_len;
        double cur_vol = sign * (ap->sample_len * init_phase * slope - volume);

        for (int i = 0; i < len; i++) {
                buf[i] = (int16_t)cur_vol;
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
int dump_waveform(audio_params_t * ap, const int16_t * stream, int len) {
        assert (ap);
        if (ap->out_fd == -1)
        	return -1;
        int n = 0;
        lseek(ap->out_fd, 0, SEEK_END);
        n = write(ap->out_fd, stream, sizeof(int16_t)*len);
        if (n == -1)
                return -1;
        ap->data_size += sizeof(int16_t)*len;
        //SIM_printf ("# Dump for waveform header.subchunk2Size=%u \n", ap->data_size);
        return 0;
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
        if (ap->wav_enable)
                if (dump_waveform ((audio_params_t *)userdata, (int16_t *)stream, len_raw / 2) == -1)
                	SIM_printf ("snd0: dump to wav file error when recording sound");       	
}

