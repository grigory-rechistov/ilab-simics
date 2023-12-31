/* audio.h - interactions with SDL for CHIP16 sound device
   Authors: Grigory Rechistov, 2014, and contributors of the MIPT Ilab project.
 */
#ifndef CHIP16_AUDIO_H
#define CHIP16_AUDIO_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Wavetypes supported by CHIP16
typedef enum {
        Audio_Meandre = 0,
        Audio_Triangle,
        Audio_Sawtooth,
        Audio_Noise,
        numb_of_wavetypes
} wave_type_t;

// Structure to control waveform currently generated by SDL callback
typedef struct audio_params {
        // host sound card parameters
        int    sample_freq;         // sampling frequency, Hz
        double sample_len;          // length of sample in seconds, == 1.0d / freq

        // common waveform parameters
        wave_type_t wave_type;      // see above
        int16_t     signal_vol;     // volume from 0 to 32767
        uint32_t    freq;           // oscillation frequency, Hz
        double      period;         // length of one oscillation in seconds,== 1.0d / signal_freq, unless zero, then it is undefined
        uint64_t    phase;          // current sample
        uint64_t    limit;          // maximum sample for this waveform

        // waveform-specific parameters
        int sign; // for meandre

        //dump to wav file parameters
        bool wav_enable;
        int out_fd;                 //file descriptor of output file
        uint32_t data_size;         //size of samples, that were played
} audio_params_t;

#pragma pack (push, 1)
typedef struct wavheader {
        //const value "RIFF"
        uint8_t chunkId[4];

        //chunkSize = 4 + (8 + subchunk1Size) + (8 + subchunk2Size)
        uint32_t chunkSize;

        //const value "WAVE"
        uint8_t format[4];

        //const value "fmt "
        uint8_t subchunk1Id[4];

        //This is the size of the rest of the Subchunk which follows this number
        //const value 16
        uint32_t subchunk1Size;

        //const value 1
        uint16_t audioFormat;

        //Mono sound now
        uint16_t numChannels;

        //Sample frequency
        uint32_t sampleRate;

        // sampleRate * numChannels * bitsPerSample/8
        uint32_t byteRate;

        // number of bytes for one sample including all channels
        // numChannels * bitsPerSample/8
        uint16_t blockAlign;

        uint16_t bitsPerSample;

        //const value "data"
        uint8_t subchunk2Id[4];

        // numSamples * numChannels * bitsPerSample/8
        uint32_t subchunk2Size;

} wavheader_t;
#pragma pack (pop)

void waveform_callback (void* userdata, uint8_t* stream, int len);


#endif // CHIP16_AUDIO_H
