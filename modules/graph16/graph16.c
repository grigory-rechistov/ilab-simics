/* graph16.c - CHIP16 graphics monitor

   This Software is part of Wind River Simics. The rights to copy, distribute,
   modify, or otherwise make use of this Software may be licensed only
   pursuant to the terms of an applicable Wind River license agreement.
   Copyright 2010-2014 Intel Corporation */

#include <simics/device-api.h>
#include <simics/devs/io-memory.h>
#include "sample-interface.h"

#include "include/SDL2/SDL.h"

#define PAL_SIZE 16

#define NIBBLE_MASK     0xF
#define UCHAR_MASK      0xFF
#define BOOLEAN_MASK    0x1

#define NEW_OP          0xAD
#define NEW_LEN         0xAD

typedef struct {
        uint8 opcode;
        uint8 length;

} graph16_instr_t;

typedef struct {
        uint8 x;
        uint8 y;
        uint16 addr;

} graph16_sprite_t;

typedef enum {
        DRW_op  = 0,
        PAL_op  = 1,
        BGC_op  = 2,
        SPR_op  = 3,
        FLIP_op = 4,

} graph16_instr_op_t;

typedef struct {
        /* Simics configuration object */
        conf_object_t obj;

        /* device specific data */
        unsigned value;

        /* registers */
        uint8   bg;             // (Nibble) Color index of background layer
        uint8   spritew;        // (Unsigned byte) Width of sprite(s) to draw
        uint8   spriteh;        // (Unsigned byte) Height of sprite(s) to draw
        bool    hflip;          // (Boolean) Flip sprite(s) to draw, horizontally
        bool    vflip;          // (Boolean) Flip sprite(s) to draw, vertically

        uint32 palette[PAL_SIZE];    // 1 colour in palette is coded like [00RRGGBB]

        graph16_sprite_t sprite;

        graph16_instr_t instruction;

        uint32 temp[24];

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

        sample->bg      = 0;
        sample->spritew = 0;
        sample->spriteh = 0;
        sample->hflip   = 0;
        sample->vflip   = 0;

        int i = 0;
        for (i = 0; i < PAL_SIZE; i++) {
                sample->palette[i] = 0;
        }

        sample->instruction.opcode = NEW_OP;
        sample->instruction.length = NEW_LEN;

        sample->sprite.x = 0;
        sample->sprite.y = 0;
        sample->sprite.addr = 0;

        for (i = 0; i < 24; i++) {
                sample->temp[i] = 0;
        }

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
        if (sample->window != NULL)
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

                SIM_LOG_INFO(1, &sample->obj, 0, "opcode = %d\n", sample->instruction.opcode);

                uint16 instr = SIM_get_mem_op_value_le(mop); // Instruction is 16 bit [XXYY]
                uint8 XX = ((instr >> 8) & 0xFF);
                uint8 YY = (instr & 0xFF);

                int tmp = 0;
                int i = 0, j = 0;
                int instr_on_getting = 0;

                if (sample->instruction.opcode == NEW_OP && sample->instruction.length == NEW_LEN) {
                        sample->instruction.opcode = XX;
                        sample->instruction.length = YY;

                        instr_on_getting = 1;
                }

                if (!instr_on_getting) {

                        switch (sample->instruction.opcode) {

                        case DRW_op:

                                sample->temp[3 - sample->instruction.length] = instr;
                                sample->instruction.length --;

                                if (sample->instruction.length == 0) {
                                        sample->sprite.x    = (sample->temp[0] & 0xFF);
                                        sample->sprite.y    = (sample->temp[1] & 0xFF);
                                        sample->sprite.addr = (sample->temp[2] & 0xFFFF);

                                        //for testing
                                        SIM_LOG_INFO(1, &sample->obj, 0, "X = %d, Y = %d, addr = %x\n",
                                                        sample->sprite.x, sample->sprite.y, sample->sprite.addr);

                                        sample->instruction.opcode = NEW_OP;
                                        sample->instruction.length = NEW_LEN;
                                }

                                break;

                        case PAL_op:
                                sample->temp[24 - sample->instruction.length] = instr;
                                sample->instruction.length --;

                                if (sample->instruction.length == 0) {
                                        j = 0;
                                        for (i = 0; i < 24; i += 3) {
                                                sample->palette[j++] = ((sample->temp[i] << 8) & 0xFFFF00) | ((sample->temp[i + 1] >> 8) & 0xFF);
                                                sample->palette[j++] = ((sample->temp[i + 1] << 16) & 0xFF0000) | (sample->temp[i + 2] & 0xFFFF);
                                        }
                                        sample->instruction.opcode = NEW_OP;
                                        sample->instruction.length = NEW_LEN;
                                        //for testing
                                        for (i = 0; i < 16; i++) {
                                                SIM_LOG_INFO(1, &sample->obj, 0, "palette[%d] = %x\n", i, sample->palette[i]);
                                        }
                                }
                                break;

                        case BGC_op:
                                sample->bg = ((XX >> 4) & 0xF);
                                sample->instruction.opcode = NEW_OP;
                                sample->instruction.length = NEW_LEN;
                                break;

                        case SPR_op:
                                sample->spritew = YY;
                                sample->spriteh = XX;
                                sample->instruction.opcode = NEW_OP;
                                sample->instruction.length = NEW_LEN;
                                break;

                        case FLIP_op:
                                tmp = ((XX >> 4) & 0xF);
                                if (tmp == 0) {
                                        sample->hflip = 0;
                                        sample->vflip = 0;
                                }
                                else if (tmp == 1) {
                                        sample->hflip = 0;
                                        sample->vflip = 1;
                                }
                                else if (tmp == 2) {
                                        sample->hflip = 1;
                                        sample->vflip = 0;
                                }
                                else if (tmp == 3) {
                                        sample->hflip = 1;
                                        sample->vflip = 1;
                                }

                                sample->instruction.opcode = NEW_OP;
                                sample->instruction.length = NEW_LEN;
                                break;

                        default:
                                SIM_LOG_INFO(1, &sample->obj, 0, "got unknown instruction: %x\n", sample->instruction.opcode);

                        }
                }
        }
        return Sim_PE_No_Exception;
}

/*
 * value attribute functions
 */
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

/*
 * bg register attribute functions
 */
static set_error_t
set_bg_attribute(void *arg, conf_object_t *obj,
                    attr_value_t *val, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        sample->bg = (NIBBLE_MASK & SIM_attr_integer(*val));
        return Sim_Set_Ok;
}

static attr_value_t
get_bg_attribute(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        return SIM_make_attr_uint64(sample->bg);
}

/*
 * spritew register attribute functions
 */
static set_error_t
set_spritew_attribute(void *arg, conf_object_t *obj,
                    attr_value_t *val, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        sample->spritew = (UCHAR_MASK & SIM_attr_integer(*val));
        return Sim_Set_Ok;
}

static attr_value_t
get_spritew_attribute(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        return SIM_make_attr_uint64(sample->spritew);
}

/*
 * spriteh register attribute functions
 */
static set_error_t
set_spriteh_attribute(void *arg, conf_object_t *obj,
                    attr_value_t *val, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        sample->spriteh = (UCHAR_MASK & SIM_attr_integer(*val));
        return Sim_Set_Ok;
}

static attr_value_t
get_spriteh_attribute(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        return SIM_make_attr_uint64(sample->spriteh);
}

/*
 * hflip register attribute functions
 */
static set_error_t
set_hflip_attribute(void *arg, conf_object_t *obj,
                    attr_value_t *val, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        sample->hflip = (BOOLEAN_MASK & SIM_attr_integer(*val));
        return Sim_Set_Ok;
}

static attr_value_t
get_hflip_attribute(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        return SIM_make_attr_uint64(sample->hflip);
}

/*
 * vflip register attribute functions
 */
static set_error_t
set_vflip_attribute(void *arg, conf_object_t *obj,
                    attr_value_t *val, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        sample->vflip = (BOOLEAN_MASK & SIM_attr_integer(*val));
        return Sim_Set_Ok;
}

static attr_value_t
get_vflip_attribute(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        return SIM_make_attr_uint64(sample->vflip);
}

/*
 * palette attribute functions
 */
static set_error_t
set_palette_attribute(void *arg, conf_object_t *obj,
                    attr_value_t *val, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;

        int i = 0, j = 0;
        for (i = 0; i < 16 * 3; i += 3, j++) {
                sample->palette[j] = 0;
                sample->palette[j] |= ((SIM_attr_integer((SIM_attr_list_item(*val, i + 0))) << 16) & 0xFF0000);
                sample->palette[j] |= ((SIM_attr_integer((SIM_attr_list_item(*val, i + 1))) << 8) & 0xFF00);
                sample->palette[j] |= ( SIM_attr_integer((SIM_attr_list_item(*val, i + 2))) & 0xFF);
        }

        //SIM_LOG_INFO(1, &sample->obj, 0, "val = %x\n", SIM_attr_integer(SIM_attr_list_item(*val, i + 0)));
        return Sim_Set_Ok;
}

static attr_value_t
get_palette_attribute(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;

        attr_value_t res = SIM_alloc_attr_list(16 * 3);
        int i = 0, j = 0;
        for (i = 0; i < 16 * 3; i += 3, j++) {
                SIM_attr_list_set_item(&res, i + 0, SIM_make_attr_uint64((sample->palette[j] >> 16) & 0xFF));
                SIM_attr_list_set_item(&res, i + 1, SIM_make_attr_uint64((sample->palette[j] >> 8 ) & 0xFF));
                SIM_attr_list_set_item(&res, i + 2, SIM_make_attr_uint64((sample->palette[j] >> 0 ) & 0xFF));
        }
        return res;
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

        SIM_register_typed_attribute(
                class, "bg",
                get_bg_attribute, NULL, set_bg_attribute, NULL,
                Sim_Attr_Optional, "i", NULL,
                "Color index of background layer.");

        SIM_register_typed_attribute(
                class, "spritew",
                get_spritew_attribute, NULL, set_spritew_attribute, NULL,
                Sim_Attr_Optional, "i", NULL,
                "Width of sprite(s) to draw.");

        SIM_register_typed_attribute(
                class, "spriteh",
                get_spriteh_attribute, NULL, set_spriteh_attribute, NULL,
                Sim_Attr_Optional, "i", NULL,
                "Height of sprite(s) to draw.");

        SIM_register_typed_attribute(
                class, "hflip",
                get_hflip_attribute, NULL, set_hflip_attribute, NULL,
                Sim_Attr_Optional, "i", NULL,
                "Flip sprite(s) to draw, horizontally.");

        SIM_register_typed_attribute(
                class, "vflip",
                get_vflip_attribute, NULL, set_vflip_attribute, NULL,
                Sim_Attr_Optional, "i", NULL,
                "Flip sprite(s) to draw, vertically.");

        SIM_register_typed_attribute(
                class, "palette",
                get_palette_attribute, NULL, set_palette_attribute, NULL,
                Sim_Attr_Optional, "[i*]", NULL,
                "The <i>palette</i>.");

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
