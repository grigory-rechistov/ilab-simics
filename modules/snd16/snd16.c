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

typedef struct {
        /* Simics configuration object */
        conf_object_t obj;

        /* device specific data */
        unsigned value;

        SDL_AudioDeviceID audiodev; // an audiodevice opened
        audio_params_t audio_params; // parameters to control waveform
} snd16_t;

/* Allocate memory for the object. */
static conf_object_t *
alloc_object(void *data)
{
        snd16_t *snd = MM_ZALLOC(1, snd16_t);
        return &snd->obj;
}

lang_void *init_object(conf_object_t *obj, lang_void *data) {
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

int delete_instance(conf_object_t *obj) {
        snd16_t *snd = (snd16_t*)obj;
        if (snd->audiodev)
                SDL_CloseAudioDevice(snd->audiodev);
        return 1;
}

static exception_type_t
operation(conf_object_t *obj, generic_transaction_t *mop, map_info_t info)
{
        snd16_t *snd = (snd16_t *)obj;
        unsigned offset = (SIM_get_mem_op_physical_address(mop)
                           + info.start - info.base);

        if (SIM_mem_op_is_read(mop)) {
                SIM_set_mem_op_value_le(mop, snd->value);
                SIM_LOG_INFO(1, &snd->obj, 0, "read from offset %d: 0x%x",
                             offset, snd->value);
        } else {
                snd->value = SIM_get_mem_op_value_le(mop);
                SIM_LOG_INFO(1, &snd->obj, 0, "write to offset %d: 0x%x",
                             offset, snd->value);
        }
        return Sim_PE_No_Exception;
}

static set_error_t
set_value_attribute(void *arg, conf_object_t *obj,
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
        else {
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
        }
        return Sim_Set_Ok;
}

static attr_value_t
get_value_attribute(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        snd16_t *snd = (snd16_t *)obj;
        return SIM_make_attr_uint64(snd->value);
}

/* called once when the device module is loaded into Simics */
void
init_local(void)
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
        SIM_register_typed_attribute(
                class, "value",
                get_value_attribute, NULL, set_value_attribute, NULL,
                Sim_Attr_Optional, "i", NULL,
                "The <i>value</i> field.");

        if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
            SIM_printf("SDL audio hasn't been initialized, doing it now\n");
            SDL_InitSubSystem(SDL_INIT_AUDIO);
        }
        // FIXME we also need to call a cleanup, maybe something like this?
        // But see this: https://developer.palm.com/distribution/viewtopic.php?f=82&t=6643
        atexit(SDL_Quit);
}
