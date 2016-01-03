#ifndef SND16_H
#define SND16_H


// States of snd_card
typedef enum snd16_states {
        State_waiting_new_op = 0,

        State_waiting_op_after_snd,   // TODO: actually snp connected with sng_advt
        State_after_freq,

        State_waiting_op_after_sng,
        State_after_advt,

} states_type_t;

typedef struct snd16 {
        /* Simics configuration object */
        conf_object_t obj;

        /* device specific data */
        // unsigned value;

        uinteger_t mop_var;
        states_type_t current_state; // switch

        SDL_AudioDeviceID audiodev; // an audiodevice opened
        audio_params_t audio_params; // parameters to control waveform

        char* out_file; //name of out wav file
        double sil_time; // to dump some silent moments we need to know their time
        bool sound_is_playing;
} snd16_t;


#endif  // SND16_H
