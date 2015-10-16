/* snd16 - CHIP16 sound device

   This Software is part of Wind River Simics. The rights to copy, distribute,
   modify, or otherwise make use of this Software may be licensed only
   pursuant to the terms of an applicable Wind River license agreement.

   Copyright 2010-2014 Intel Corporation */

#include <simics/device-api.h>
#include <simics/devs/io-memory.h>
#include "sample-interface.h"
#include <simics/simulator-api.h> // For SIM_printf()

#include "include/SDL2/SDL.h"
#include "audio.h"
#include "snd16.h"

#define SDL_VOL       16000
#define SND_WAVE_TYPE Audio_Meandre

#define SND 0x0002
#define SNG 0x0102


static attr_value_t
get_wave_type (void *arg, conf_object_t *obj, attr_value_t *idx);
static set_error_t
set_wave_type (void *arg, conf_object_t *obj, attr_value_t *val, attr_value_t *idx);

static attr_value_t
get_signal_freq (void *arg, conf_object_t *obj, attr_value_t *idx);
static set_error_t
set_signal_freq (void *arg, conf_object_t *obj, attr_value_t *val, attr_value_t *idx);

static attr_value_t
get_waveform_limit (void *arg, conf_object_t *obj, attr_value_t *idx);
static set_error_t
set_waveform_limit (void *arg, conf_object_t *obj, attr_value_t *val, attr_value_t *idx);

/* THREAD_SAFE_GLOBAL: pause_snd_event init */
static event_class_t* pause_snd_event;
static void snd_mute (conf_object_t *obj, void *param);

/* Allocate memory for the object. */
static conf_object_t *
alloc_object (void *data)
        {
        snd16_t *snd = MM_ZALLOC(1, snd16_t);
        return &snd->obj;
        }

lang_void *init_object(conf_object_t *obj, lang_void *data)
        {
        snd16_t *snd = (snd16_t*)obj;

        SDL_AudioSpec want;
        SDL_zero(want);
        want.freq = 44100;
        want.format = AUDIO_S16;
        want.channels = 1; /* mono sound */
        want.samples = 4096;
        want.callback = waveform_callback;
        want.userdata = (void*)&snd->audio_params;
        snd->audio_params.freq = want.freq;
        snd->audio_params.sample = 1.0d / want.freq;
        snd->audiodev = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
        if (snd->audiodev == 0)
                SIM_LOG_INFO(1, obj, 0, "Failed to open audio device: %s", SDL_GetError());
//        SIM_LOG_INFO(1, obj, 0, "Audio device is %s", SDL_GetAudioDeviceName(0, 0)); // incorrect, don't know if possible at all // GGG
        return obj;
        }

int delete_instance (conf_object_t *obj)
        {
        snd16_t *snd = (snd16_t*)obj;
        if (snd->audiodev)
                SDL_CloseAudioDevice(snd->audiodev);
        return 1;
        }

static exception_type_t
operation (conf_object_t *obj, generic_transaction_t *mop, map_info_t info)
        {
        snd16_t *snd = (snd16_t *)obj;
        conf_object_t* queue_obj = SIM_attr_object (SIM_get_attribute(&snd->obj, "queue"));
                ASSERT (queue_obj != NULL);
        unsigned offset = (SIM_get_mem_op_physical_address(mop) + info.start - info.base);

        if (SIM_mem_op_is_write (mop))
                {
                if (offset != 0)
                        {
                        SIM_LOG_ERROR(&snd->obj, 0, "mop offset != 0, but = %d", offset);
                        return Sim_PE_IO_Error;
                        }

                snd->mop_var = SIM_get_mem_op_value_le(mop);      // le - little endian
                attr_value_t mop_var_attr = SIM_make_attr_uint64(0);
                double mop_var_double = (double) snd->mop_var;
                set_error_t ret_val = 0;

                switch (snd->current_state)
                {
                case State_waiting_new_op:
                        if (snd->mop_var == SND)
                                {
                                switch (SND_WAVE_TYPE)
                                {
                                case Audio_Meandre:
                                        snd->audio_params.wave_type = Audio_Meandre;
                                        snd->audio_params.sign      = 1;
                                        break;

                                case Audio_Noise:
                                        snd->audio_params.wave_type = Audio_Noise;
                                        break;

                                default:
                                        ASSERT(!"wrong debug define wave_type");
                                }

                                snd->audio_params.sdl_vol   = SDL_VOL;
                                snd->audio_params.phase     = 0;

                                snd->current_state = State_waiting_op_after_snd;
                                goto return_ok;
                                }

                        if (snd->mop_var == SNG)
                                {
                                SIM_LOG_ERROR(&snd->obj, 0, "snd0: not realised yet");
                                // snd->current_state = State_waiting_op_after_sng;
                                // goto return_ok;
                                }

                        SIM_LOG_ERROR(&snd->obj, 0, "snd0: unknown instruction");
                        return Sim_PE_IO_Error;
                        break;

                case State_waiting_op_after_snd:
                        mop_var_attr = SIM_make_attr_uint64(snd->mop_var);
                        ret_val = set_signal_freq (NULL, &snd->obj, &mop_var_attr, NULL);
                        if (ret_val == Sim_Set_Ok)
                                {
                                if (snd->mop_var != 0)
                                        snd->audio_params.period = 1.0d / snd->audio_params.signal_freq;
                                else
                                        snd->audio_params.period = -1;

                                snd->current_state = State_after_freq;
                                goto return_ok;
                                }

                        SIM_LOG_ERROR(&snd->obj, 0, "snd0: smth wrong with State_waiting_op_after_snd");
                        return Sim_PE_IO_Error;
                        break;

                case State_after_freq:
                        mop_var_attr = SIM_make_attr_uint64(snd->mop_var);
                        ret_val = set_waveform_limit (NULL, &snd->obj, &mop_var_attr, NULL);
                        if (ret_val == Sim_Set_Ok)
                                {
                                SIM_event_cancel_time(queue_obj, pause_snd_event, &snd->obj, NULL, NULL);
                                if (snd->audio_params.signal_freq == 0)
                                        SDL_PauseAudioDevice(snd->audiodev, 1);
                                else
                                        {
                                        // here should be launch of sound generating
                                        SDL_PauseAudioDevice(snd->audiodev, 0);
                                        // SDL_Delay(snd->mop_var);
                                        // SDL_PauseAudioDevice(snd->audiodev, 1);
                                        SIM_event_post_time (queue_obj, pause_snd_event, &snd->obj, mop_var_double / 1000, NULL);
                                        }

                                snd->current_state = State_waiting_new_op;
                                goto return_ok;
                                }

                        SIM_LOG_ERROR(&snd->obj, 0, "snd0: smth wrong with State_after_freq");
                        return Sim_PE_IO_Error;
                        break;

                case State_waiting_op_after_sng:
                        SIM_LOG_ERROR(&snd->obj, 0, "snd0: not realised yet");
                        break;

                case State_after_advt:
                        SIM_LOG_ERROR(&snd->obj, 0, "snd0: not realised yet");
                        break;

                default:
                        ASSERT(0);
                } // end of switch
                } // end of if (SIM_mem_op_is_write (mop)), before else

        else
                SIM_LOG_SPEC_VIOLATION (1, &snd->obj, 0, "snd memory is write-only");

return_ok:
        return Sim_PE_No_Exception;
        }

//------------------------------------------------------------------------------
// Getters and setters
//------------------------------------------------------------------------------
/*static attr_value_t
get_value_attribute (void *arg, conf_object_t *obj, attr_value_t *idx)
        {
        snd16_t *snd = (snd16_t *)obj;
        return SIM_make_attr_uint64(snd->value);
        }

static set_error_t
set_value_attribute (void *arg, conf_object_t *obj,
                    attr_value_t *val, attr_value_t *idx)
        {
        snd16_t *snd = (snd16_t *)obj;
        snd->value = SIM_attr_integer(*val);

        if (!snd->audiodev)
                return Sim_Set_Ok;

        // For debug purposes, we can write to this attribute to force
        // sound generation.
        if (snd->value == 0)
                SDL_PauseAudioDevice(snd->audiodev, 1);
        else
                {
                SDL_PauseAudioDevice(snd->audiodev, 0);
                */
                /*
                SDL_PauseAudioDevice(snd->audiodev, 1);
                // prepare meandre parameters
                snd->audio_params.wave_type = Audio_Meandre;
                snd->audio_params.sdl_vol = 16000;
                snd->audio_params.signal_freq = 440;
                snd->audio_params.period = 1.0d / snd->audio_params.signal_freq;
                snd->audio_params.phase = 0;
                snd->audio_params.limit = 7000;
                snd->audio_params.sign = 1;

                SDL_PauseAudioDevice(snd->audiodev, 0);
                SDL_Delay(500);
                SDL_PauseAudioDevice(snd->audiodev, 1);
                */
                /*
                }

        return Sim_Set_Ok;
        }
*/

static attr_value_t
get_wave_type (void *arg, conf_object_t *obj, attr_value_t *idx)
        {
        snd16_t *snd = (snd16_t *)obj;
        wave_type_t tmp = snd->audio_params.wave_type;
        return SIM_make_attr_uint64(tmp);
        }

static set_error_t
set_wave_type (void *arg, conf_object_t *obj,
                          attr_value_t *val, attr_value_t *idx)
        {
        snd16_t *snd = (snd16_t *)obj;
        uint16 new_wave_type = SIM_attr_integer(*val);

        set_error_t ret = Sim_Set_Ok;

        if ((0 <= new_wave_type) && (new_wave_type < numb_of_wavetypes))
                snd->audio_params.wave_type = new_wave_type;
        else
                ret = Sim_Set_Illegal_Value;

        return ret;
        }

static attr_value_t
get_signal_freq (void *arg, conf_object_t *obj, attr_value_t *idx)
        {
        snd16_t *snd = (snd16_t *)obj;
        uint32_t tmp = snd->audio_params.signal_freq;
        return SIM_make_attr_uint64(tmp);
        }

static set_error_t
set_signal_freq (void *arg, conf_object_t *obj,
                          attr_value_t *val, attr_value_t *idx)
        {
        snd16_t *snd = (snd16_t *)obj;
        uint32_t new_signal_freq = SIM_attr_integer(*val);

        set_error_t ret = Sim_Set_Ok;

        if ((0 <= new_signal_freq) && (new_signal_freq < UINT32_MAX))
                snd->audio_params.signal_freq = new_signal_freq;
        else
                ret = Sim_Set_Illegal_Value;

        return ret;
        }

static attr_value_t
get_waveform_limit (void *arg, conf_object_t *obj, attr_value_t *idx)
        {
        snd16_t *snd = (snd16_t *)obj;
        uint64_t tmp = snd->audio_params.limit;
        return SIM_make_attr_uint64(tmp);
        }

static set_error_t
set_waveform_limit (void *arg, conf_object_t *obj,
                          attr_value_t *val, attr_value_t *idx)
        {
        snd16_t *snd = (snd16_t *)obj;
        uint64_t new_limit = SIM_attr_integer(*val);

        set_error_t ret = Sim_Set_Ok;

        if ((0 <= new_limit) && (new_limit < UINT64_MAX))
                snd->audio_params.limit = new_limit;
        else
                ret = Sim_Set_Illegal_Value;

        return ret;
        }

static void snd_mute (conf_object_t *obj, void *param)
        {
        snd16_t *snd = (snd16_t *)obj;
        SDL_PauseAudioDevice(snd->audiodev, 1);
        SIM_LOG_INFO (4, obj, 0, "sdl_mute was called");
        }
//------------------------------------------------------------------------------

/* called once when the device module is loaded into Simics */
void
init_local (void)
        {
        /* Register the class with callbacks used when creating and deleting
           new instances of the class */
        const class_data_t funcs = {
                .alloc_object = alloc_object,
                .init_object  = init_object,
                .delete_instance = delete_instance,
                .class_desc = "CHIP16 sound device",
                .description =
                        "Sound device for CHIP16 platform"
                };
        conf_class_t *class = SIM_register_class("snd16", &funcs);

        /* Register the 'io_memory' interface, which is an example of a generic
           interface that is implemented by all memory mapped devices. */
        static const io_memory_interface_t memory_iface = {
                .operation = operation
                };
        SIM_register_interface(class, IO_MEMORY_INTERFACE, &memory_iface);

        /* Register attributes (device specific data) together with functions
           for getting and setting these attributes. */
        /*
        SIM_register_typed_attribute(
                class, "value",
                get_value_attribute, NULL, set_value_attribute, NULL,
                Sim_Attr_Optional, "i", NULL,
                "The <i>value</i> field.");
                */

        SIM_register_typed_attribute(
                class, "wave_type",
                get_wave_type, NULL,
                set_wave_type, NULL,
                Sim_Attr_Optional,
                "i", NULL,
                "Type of wave: Meandre, Triangle, Sawtooth, Pulse, Noise.");

        SIM_register_typed_attribute(
                class, "signal_freq",
                get_signal_freq, NULL,
                set_signal_freq, NULL,
                Sim_Attr_Optional,
                "i", NULL,
                "Frequency of signal.");

        SIM_register_typed_attribute(
                class, "limit",
                get_waveform_limit, NULL,
                set_waveform_limit, NULL,
                Sim_Attr_Optional,
                "i", NULL,
                "Limit - maximum sample for this waveform.");

        if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
                SIM_printf("SDL audio hasn't been initialized, doing it now\n");
                SDL_InitSubSystem(SDL_INIT_AUDIO);
                }


        pause_snd_event = SIM_register_event(
                                "Mute sound", class,
                                0 /*Sim_EC_Notsaved*/, snd_mute,
                                0, 0, 0, 0);
        ASSERT (pause_snd_event != NULL);
        }
