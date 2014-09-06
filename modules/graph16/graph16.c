/* sample-device.c - sample code for a Simics device

   This Software is part of Wind River Simics. The rights to copy, distribute,
   modify, or otherwise make use of this Software may be licensed only
   pursuant to the terms of an applicable Wind River license agreement.
  
   Copyright 2010-2014 Intel Corporation */

#include <simics/device-api.h>
#include <simics/devs/io-memory.h>
#include "sample-interface.h"

#include "include/SDL2/SDL.h"

typedef struct {
        /* Simics configuration object */
        conf_object_t obj;

        /* device specific data */
        unsigned value;

        SDL_Window *window;

} graph16_t;

/* Allocate memory for the object. */
static conf_object_t *
alloc_object(void *data)
{
        graph16_t *sample = MM_ZALLOC(1, graph16_t);
        return &sample->obj;
}

lang_void *init_object(conf_object_t *obj, lang_void *data) {
        graph16_t *sample = (graph16_t *)obj;

        sample->window = SDL_CreateWindow (
        "CHIP16 monitor",                  // window title, TODO include SIM_object_name output
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        320,                               // width, in pixels
        240,                               // height, in pixels
        SDL_WINDOW_SHOWN                   // flags 
        );

        if (sample->window == NULL) {
                SIM_LOG_INFO(1, obj, 0, "Failed to create SDL window");
        }

        return obj;
}

int delete_instance(conf_object_t *obj) {
        graph16_t *sample = (graph16_t *)obj;

        SDL_DestroyWindow(sample->window);
        return 1;
}

/* Dummy function that doesn't really do anything. */
static void
simple_method(conf_object_t *obj, int arg)
{
        graph16_t *sample = (graph16_t *)obj;
        SIM_LOG_INFO(1, &sample->obj, 0,
                     "'simple_method' called with arg %d", arg);
}

static exception_type_t
operation(conf_object_t *obj, generic_transaction_t *mop,
                 map_info_t info)
{
        graph16_t *sample = (graph16_t *)obj;
        unsigned offset = (SIM_get_mem_op_physical_address(mop)
                           + info.start - info.base);

        if (SIM_mem_op_is_read(mop)) {
                SIM_set_mem_op_value_le(mop, sample->value);
                SIM_LOG_INFO(1, &sample->obj, 0, "read from offset %d: 0x%x",
                             offset, sample->value);
        } else {
                sample->value = SIM_get_mem_op_value_le(mop);
                SIM_LOG_INFO(1, &sample->obj, 0, "write to offset %d: 0x%x",
                             offset, sample->value);
        }
        return Sim_PE_No_Exception;
}

static set_error_t
set_value_attribute(void *arg, conf_object_t *obj,
                    attr_value_t *val, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        sample->value = SIM_attr_integer(*val);
        return Sim_Set_Ok;
}

static attr_value_t
get_value_attribute(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        return SIM_make_attr_uint64(sample->value);
}

static set_error_t
set_add_log_attribute(void *arg, conf_object_t *obj, attr_value_t *val,
                      attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        SIM_LOG_INFO(1, &sample->obj, 0, "%s", SIM_attr_string(*val));
        return Sim_Set_Ok;
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
                .class_desc = "CHIP16 video device",
                .description =
                "Video device for CHIP16 platform"
        };
        conf_class_t *class = SIM_register_class("graph16", &funcs);

        /* Register the 'sample-interface', which is an example of a unique,
           customized interface that we've implemented for this device. */
        static const sample_interface_t sample_iface = {
                .simple_method = simple_method
        };
        SIM_register_interface(class, SAMPLE_INTERFACE, &sample_iface);

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

        /* Pseudo attribute, not saved in configuration */
        SIM_register_typed_attribute(
                class, "add_log",
                0, NULL, set_add_log_attribute, NULL,
                Sim_Attr_Pseudo, "s", NULL,
                "<i>Write-only</i>. Strings written to this"
                " attribute will end up in the device's log file.");

        SDL_Init(SDL_INIT_VIDEO);
        // FIXME we also need to call a cleanup, maybe something like this?
        // But see this: https://developer.palm.com/distribution/viewtopic.php?f=82&t=6643
        atexit(SDL_Quit);

} // init_local()
