/* graph16.c - CHIP16 graphics monitor

   This Software is part of Wind River Simics. The rights to copy, distribute,
   modify, or otherwise make use of this Software may be licensed only
   pursuant to the terms of an applicable Wind River license agreement.
   Copyright 2010-2014 Intel Corporation */

#include "graph16.h"
#include "sample-interface.h"

#include <simics/device-api.h>
#include <simics/devs/io-memory.h>

#include "include/SDL2/SDL.h"


/* Allocate memory for the object. */
static conf_object_t *
alloc_object(void *data)
{
        graph16_t *sample = MM_ZALLOC(1, graph16_t);
        return &sample->obj;
}

lang_void *init_object(conf_object_t *obj, lang_void *data)
{
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

        sample->instruction.opcode = 0;
        sample->instruction.length = 0;

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

int delete_instance(conf_object_t *obj)
{
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

                SIM_LOG_INFO(4, &sample->obj, 0, "opcode = %d\n", sample->instruction.opcode);

                uint16 instr = SIM_get_mem_op_value_le(mop); // Instruction is 16 bit [XXYY]
                uint8 XX = ((instr >> 8) & 0xFF);
                uint8 YY = (instr & 0xFF);

                int tmp = 0;
                int i = 0, j = 0;

                graph16_sprite_t sprite;
                sprite.x = 0;
                sprite.y = 0;
                sprite.addr = 0;

                if (sample->instruction.length == 0) {
                        sample->instruction.opcode = XX;
                        sample->instruction.length = YY;
                } else {


                        switch (sample->instruction.opcode) {

                        case DRW_op:

                                sample->temp[3 - sample->instruction.length] = instr;
                                sample->instruction.length--;

                                if (sample->instruction.length == 0) {

                                        sprite.x    = (sample->temp[0] & 0xFF);
                                        sprite.y    = (sample->temp[1] & 0xFF);
                                        sprite.addr = (sample->temp[2] & 0xFFFF);

                                        graph16_draw_sprite (sample, &sprite);

                                        SIM_LOG_INFO(4, &sample->obj, 0, "DRW: X = %d, Y = %d, addr = %x\n",
                                                                                        sprite.x,
                                                                                        sprite.y,
                                                                                        sprite.addr);                                        //for testing
                                }

                                break;

                        case PAL_op:

                                sample->temp[24 - sample->instruction.length] = instr;
                                sample->instruction.length--;

                                if (sample->instruction.length == 0) {
                                        j = 0;
                                        for (i = 0; i < 24; i += 3) {  // PAL command is transmitted in 24 transactions
                                                sample->palette[j++] = ((sample->temp[i] << 8) & 0xFFFF00) | ((sample->temp[i + 1] >> 8) & 0xFF);
                                                sample->palette[j++] = ((sample->temp[i + 1] << 16) & 0xFF0000) | (sample->temp[i + 2] & 0xFFFF);
                                        }

                                        for (i = 0; i < PAL_SIZE; i++) {
                                                SIM_LOG_INFO(4, &sample->obj, 0, "palette[%d] = %x\n", i, sample->palette[i]);
                                        }
                                }
                                break;

                        case BGC_op:

                                sample->bg = ((XX >> 4) & 0xF);
                                sample->instruction.length--;
                                break;

                        case SPR_op:

                                sample->spritew = YY;
                                sample->spriteh = XX;

                                sample->instruction.length--;
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

                                else  {
                                        ASSERT (0);
                                }

                                sample->instruction.length--;
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
        sample->bg = (0xF & SIM_attr_integer(*val));
        return Sim_Set_Ok;
}

static attr_value_t
get_bg_attribute(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        return SIM_make_attr_uint64(sample->bg);
}


/*
 * physical_memory_space attribute functions
 */

static attr_value_t
graph16_get_physical_memory(void *arg, conf_object_t *obj,
                                attr_value_t *idx)
{
        graph16_t *core = conf_to_graph16(obj);
        return SIM_make_attr_object(core->physical_mem_obj);
}

static set_error_t
graph16_set_physical_memory(void *arg, conf_object_t *obj,
                                attr_value_t *val, attr_value_t *idx)
{
        graph16_t *core = conf_to_graph16(obj);

        const memory_space_interface_t *mem_space_iface;
        conf_object_t *oval = SIM_attr_object(*val);
        mem_space_iface = (memory_space_interface_t *)SIM_c_get_interface(
                oval, MEMORY_SPACE_INTERFACE);

        memory_page_interface_t *mem_page_iface;
        mem_page_iface = (memory_page_interface_t *)SIM_c_get_interface(
                oval, MEMORY_PAGE_INTERFACE);

        breakpoint_trigger_interface_t *bp_trig_iface;
        bp_trig_iface = (breakpoint_trigger_interface_t *)SIM_c_get_interface(
                oval, BREAKPOINT_TRIGGER_INTERFACE);

        if (!mem_space_iface || !mem_page_iface || !bp_trig_iface) {
                return Sim_Set_Interface_Not_Found;
        }

        core->physical_mem_obj = oval;

        core->physical_mem_space_iface = mem_space_iface;
        core->physical_mem_page_iface = mem_page_iface;
        core->physical_mem_bp_trig_iface = bp_trig_iface;

        return Sim_Set_Ok;
}

/*
 * video_memory_space attribute functions
 */

static attr_value_t
graph16_get_video_memory(void *arg, conf_object_t *obj,
                                attr_value_t *idx)
{
        graph16_t *core = conf_to_graph16(obj);
        return SIM_make_attr_object(core->video_mem_obj);
}

static set_error_t
graph16_set_video_memory(void *arg, conf_object_t *obj,
                                attr_value_t *val, attr_value_t *idx)
{
        graph16_t *core = conf_to_graph16(obj);

        const memory_space_interface_t *mem_space_iface;
        conf_object_t *oval = SIM_attr_object(*val);
        mem_space_iface = (memory_space_interface_t *)SIM_c_get_interface(
                oval, MEMORY_SPACE_INTERFACE);

        memory_page_interface_t *mem_page_iface;
        mem_page_iface = (memory_page_interface_t *)SIM_c_get_interface(
                oval, MEMORY_PAGE_INTERFACE);

        breakpoint_trigger_interface_t *bp_trig_iface;
        bp_trig_iface = (breakpoint_trigger_interface_t *)SIM_c_get_interface(
                oval, BREAKPOINT_TRIGGER_INTERFACE);

        if (!mem_space_iface || !mem_page_iface || !bp_trig_iface) {
                return Sim_Set_Interface_Not_Found;
        }

        core->video_mem_obj = oval;

        core->video_mem_space_iface = mem_space_iface;
        core->video_mem_page_iface = mem_page_iface;
        core->video_mem_bp_trig_iface = bp_trig_iface;

        return Sim_Set_Ok;
}

/*
 * spritew register attribute functions
 */
static set_error_t
set_spritew_attribute(void *arg, conf_object_t *obj,
                    attr_value_t *val, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        sample->spritew = (0xFF & SIM_attr_integer(*val));
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
        sample->spriteh = (0xFF & SIM_attr_integer(*val));
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
        sample->hflip = SIM_attr_boolean(*val);
        return Sim_Set_Ok;
}

static attr_value_t
get_hflip_attribute(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        return SIM_make_attr_boolean(sample->hflip);
}

/*
 * vflip register attribute functions
 */
static set_error_t
set_vflip_attribute(void *arg, conf_object_t *obj,
                    attr_value_t *val, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        sample->vflip = SIM_attr_boolean(*val);;
        return Sim_Set_Ok;
}

static attr_value_t
get_vflip_attribute(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;
        return SIM_make_attr_boolean(sample->vflip);
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
        for (i = 0; i < PAL_SIZE * 3; i += 3, j++) {
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

        attr_value_t res = SIM_alloc_attr_list(PAL_SIZE * 3);
        int i = 0, j = 0;
        for (i = 0; i < PAL_SIZE * 3; i += 3, j++) {
                SIM_attr_list_set_item(&res, i + 0, SIM_make_attr_uint64((sample->palette[j] >> 16) & 0xFF));
                SIM_attr_list_set_item(&res, i + 1, SIM_make_attr_uint64((sample->palette[j] >> 8 ) & 0xFF));
                SIM_attr_list_set_item(&res, i + 2, SIM_make_attr_uint64((sample->palette[j] >> 0 ) & 0xFF));
        }
        return res;
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
                class, "physical_memory_space",
                graph16_get_physical_memory, NULL,
                graph16_set_physical_memory, NULL,
                Sim_Attr_Required,
                "o", NULL,
                "Physical memory video space.");

        SIM_register_typed_attribute(
                class, "video_memory_space",
                graph16_get_video_memory, NULL,
                graph16_set_video_memory, NULL,
                Sim_Attr_Required,
                "o", NULL,
                "Physical memory video space.");


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
                Sim_Attr_Optional, "b", NULL,
                "Flip sprite(s) to draw, horizontally.");

        SIM_register_typed_attribute(
                class, "vflip",
                get_vflip_attribute, NULL, set_vflip_attribute, NULL,
                Sim_Attr_Optional, "b", NULL,
                "Flip sprite(s) to draw, vertically.");

        SIM_register_typed_attribute(
                class, "palette",
                get_palette_attribute, NULL, set_palette_attribute, NULL,
                Sim_Attr_Optional, "[i*]", NULL,
                "The <i>palette</i>.");

        SDL_Init(SDL_INIT_VIDEO);
        // FIXME we also need to call a cleanup, maybe something like this?
        // But see this: https://developer.palm.com/distribution/viewtopic.php?f=82&t=6643
        atexit(SDL_Quit);

} // init_local()

static generic_transaction_t
create_generic_transaction (conf_object_t *initiator, mem_op_type_t type,
                           physical_address_t phys_address,
                           physical_address_t len, uint8 *data,
                           endianness_t endian)
{
        generic_transaction_t ret;
        memset(&ret, 0, sizeof(ret));
        ret.logical_address = 0;
        ret.physical_address = phys_address;
        ret.real_address = data;
        ret.size = len;
        ret.ini_type = Sim_Initiator_CPU;
        ret.ini_ptr = initiator;
        ret.use_page_cache = 1;
        ret.inquiry = 0;
        SIM_set_mem_op_type(&ret, type);

#if defined(HOST_LITTLE_ENDIAN)
        if (endian == Sim_Endian_Host_From_BE)
                ret.inverse_endian = 1;
#else
        if (endian == Sim_Endian_Host_From_LE)
                ret.inverse_endian = 1;
#endif

        return ret;
}

bool
graph16_write_memory (graph16_t *core, int mem_switch, physical_address_t phys_address,
                    physical_address_t len, uint8 *data)
{

        conf_object_t *mem_obj = NULL;
        const memory_space_interface_t *mem_space_iface = NULL;

        if (mem_switch == PHYS_MEM) {

                mem_obj = core->physical_mem_obj;
                mem_space_iface = core->physical_mem_space_iface;
        }
        else if (mem_switch == VIDEO_MEM) {

                mem_obj = core->video_mem_obj;
                mem_space_iface = core->video_mem_space_iface;
        }
        else {
                // We'll never be here. Maybe.
                ASSERT (0);
        }

        generic_transaction_t mem_op = create_generic_transaction(
                graph16_to_conf(core),
                Sim_Trans_Store,
                phys_address, len,
                data,
                Sim_Endian_Target);

        exception_type_t exc =
             mem_space_iface->access(mem_obj, &mem_op);

        if (exc != Sim_PE_No_Exception) {
                SIM_LOG_ERROR(graph16_to_conf(core), 0,
                              "write error to physical address 0x%llx",
                              phys_address);
                return false;
        }

        return true;
}

bool
graph16_read_memory (graph16_t *core, int mem_switch, physical_address_t phys_address,
                   physical_address_t len, uint8 *data)
{
        conf_object_t *mem_obj = NULL;
        const memory_space_interface_t *mem_space_iface = NULL;

        if (mem_switch == PHYS_MEM) {

                mem_obj = core->physical_mem_obj;
                mem_space_iface = core->physical_mem_space_iface;
        }
        else if (mem_switch == VIDEO_MEM) {

                mem_obj = core->video_mem_obj;
                mem_space_iface = core->video_mem_space_iface;
        }        else {
                // We'll never be here. Maybe.
                ASSERT (0);
        }

        generic_transaction_t mem_op = create_generic_transaction(
                graph16_to_conf(core),
                Sim_Trans_Load,
                phys_address, len,
                data,
                Sim_Endian_Target);

        exception_type_t exc =
             mem_space_iface->access(mem_obj, &mem_op);

        if (exc != Sim_PE_No_Exception) {
                SIM_LOG_ERROR(graph16_to_conf(core), 0,
                             "read error from physical address 0x%llx",
                              phys_address);
                return false;
        }

        return true;
}

int
graph16_draw_sprite (graph16_t *core, graph16_sprite_t *sprite)
{
        int ret = 0;
        int sprite_size = core->spriteh * core->spritew;

        uint8 data[sprite_size];

        ret = graph16_read_memory (core, PHYS_MEM, sprite->addr,
                                   sprite_size, data);
        if (ret == false) {
                SIM_LOG_ERROR(graph16_to_conf(core), 0,
                             "failed to read sprite from phys memory");
                return false;
        }

        ret = graph16_write_memory (core, VIDEO_MEM, 0x00,
                                    sprite_size, data);

        if (ret == false) {
                SIM_LOG_ERROR(graph16_to_conf(core), 0,
                             "failed to write sprite to video memory");
                return false;
        }

        return true;
}

