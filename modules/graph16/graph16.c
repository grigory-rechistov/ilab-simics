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
#include <string.h>

const graph16_pal_item default_palette[PAL_SIZE] = {{0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x88, 0x88, 0x88}, {0xBF, 0x39, 0x32},
                                                    {0xDE, 0x7A, 0xAE}, {0x4C, 0x3D, 0x21}, {0x90, 0x5F, 0x25}, {0xE4, 0x94, 0x52},
                                                    {0xEA, 0xD9, 0x79}, {0x53, 0x7A, 0x3B}, {0xAB, 0xD5, 0x4A}, {0x25, 0x2E, 0x38},
                                                    {0x00, 0x46, 0x7F}, {0x68, 0xAB, 0xCC}, {0xBC, 0xDE, 0xE4}, {0xFF, 0xFF, 0xFF}};

static generic_transaction_t create_generic_transaction (conf_object_t *initiator, mem_op_type_t type,
                           physical_address_t phys_address,
                           physical_address_t len, uint8 *data,
                           endianness_t endian);

static int graph16_refresh_screen(void *arg);

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


        // Initializing palette with default colours
        memcpy (sample->palette, default_palette, sizeof(default_palette));

        sample->instruction.opcode = 0;
        sample->instruction.length = 0;

        for (i = 0; i < 24; i++) {
                sample->temp[i] = 0;
        }


        sample->window = SDL_CreateWindow (
                SIM_object_name(obj),                  // window title, TODO include SIM_object_name output
                SDL_WINDOWPOS_UNDEFINED,           // initial x position
                SDL_WINDOWPOS_UNDEFINED,           // initial y position
                SCREEN_W,                          // width, in pixels
                SCREEN_H,                          // height, in pixels
                SDL_WINDOW_SHOWN                   // flags
        );
        if (sample->window == NULL) {
                SIM_LOG_INFO(1, obj, 0, "Failed to create SDL window: %s", SDL_GetError());
                goto end;
        }

        sample->renderer = SDL_CreateRenderer (sample->window, 0, SDL_RENDERER_ACCELERATED);
        if (sample->renderer == NULL) {
                SIM_LOG_ERROR(obj, 0, "Failed to create SDL renderer: %s", SDL_GetError());
                goto end;
        }

        sample->texture = SDL_CreateTexture (sample->renderer,
                                             SDL_PIXELFORMAT_ARGB8888,    // Format of 32bit ARGB color
                                             SDL_TEXTUREACCESS_STATIC,
                                             SCREEN_W,
                                             SCREEN_H
        );
        if (sample->texture == NULL) {
                SIM_LOG_ERROR(obj, 0, "Failed to create SDL texture: %s", SDL_GetError());
                goto end;
        }

        sample->screen = SDL_CreateRGBSurface (0,
                                               SCREEN_W,
                                               SCREEN_H,
                                               32,
                                               0,
                                               0,
                                               0,
                                               0
        );
        if (sample->screen == NULL) {
                SIM_LOG_ERROR(obj, 0, "Failed to create SDL surface (screen): %s", SDL_GetError());
                goto end;
        }

        /* Create a separate thread to refresh SDL GUI window */
        sample->refresh_active = true;
        sample->refresh_thread = SDL_CreateThread(graph16_refresh_screen, "graph16 refresh screen thread", (void *)sample);
        if (sample->refresh_thread == NULL) {
                SIM_LOG_ERROR(obj, 0, "Failed to create SDL thread: %s", SDL_GetError());
                goto end;
        }

        end:

        return obj;
}

int delete_instance(conf_object_t *obj)
{
        graph16_t *sample = (graph16_t *)obj;

        if (sample->window != NULL)
                SDL_DestroyWindow(sample->window);

        if (sample->screen != NULL)
                SDL_FreeSurface(sample->screen);

        if (sample->renderer != NULL)
                SDL_DestroyRenderer(sample->renderer);


        if (sample->texture != NULL)
                SDL_DestroyTexture(sample->texture);

        /* Shut down the GUI refresh thread */
        sample->refresh_active = false;

        /* A memory barrier would be nice here */
        SIM_LOG_INFO(4, obj, 0, "Waiting for the refresh thread to return...");
        SDL_WaitThread(sample->refresh_thread, NULL);

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

                                        sprite.x    = (sample->temp[0] & 0xFFFF);
                                        sprite.y    = (sample->temp[1] & 0xFFFF);
                                        sprite.addr = (sample->temp[2] & 0xFFFF);

                                        SIM_LOG_INFO(4, &sample->obj, 0, "DRW: X = %d, Y = %d, addr = %x\n",
                                                                                        sprite.x,
                                                                                        sprite.y,
                                                                                        sprite.addr);

                                        graph16_draw_sprite (sample, &sprite);
                                        graph16_update_screen (sample);

                                }

                                break;

                        case PAL_op:

                                sample->temp[24 - sample->instruction.length] = instr;
                                sample->instruction.length--;

                                if (sample->instruction.length == 0) {
                                        j = 0;
                                        for (i = 0; i < 24; i += 3) {  // PAL command is transmitted in 24 transactions
                                                sample->palette[j].R = (sample->temp[i] >> 8) & 0xFF;
                                                sample->palette[j].G =  sample->temp[i] & 0xFF;
                                                sample->palette[j].B = (sample->temp[i + 1] >> 8) & 0xFF;
                                                j++;

                                                sample->palette[j].R =  sample->temp[i + 1] & 0xFF;
                                                sample->palette[j].G = (sample->temp[i + 2] >> 8) & 0xFF;
                                                sample->palette[j].B =  sample->temp[i + 2] & 0xFF;
                                                j++;
                                        }

                                        for (i = 0; i < PAL_SIZE; i++) {
                                                SIM_LOG_INFO(4, &sample->obj, 0, "palette[%d] = %x, %x, %x\n",
                                                                                                i,
                                                                                                sample->palette[i].R,
                                                                                                sample->palette[i].G,
                                                                                                sample->palette[i].B);
                                        }
                                }
                                break;

                        case BGC_op:

                                sample->bg = ((XX >> 4) & 0xF);
                                sample->instruction.length--;
                                break;

                        case SPR_op:

                                sample->spritew = XX;
                                sample->spriteh = YY;

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

                sample->palette[j].R = ((SIM_attr_integer((SIM_attr_list_item(*val, i + 0))) << 16) & 0xFF0000);
                sample->palette[j].G = ((SIM_attr_integer((SIM_attr_list_item(*val, i + 1))) << 8) & 0xFF00);
                sample->palette[j].B = ( SIM_attr_integer((SIM_attr_list_item(*val, i + 2))) & 0xFF);
        }

        return Sim_Set_Ok;
}

static attr_value_t
get_palette_attribute(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        graph16_t *sample = (graph16_t *)obj;

        attr_value_t res = SIM_alloc_attr_list(PAL_SIZE * 3);

        int i = 0, j = 0;
        for (i = 0; i < PAL_SIZE * 3; i += 3, j++) {
                SIM_attr_list_set_item(&res, i + 0, SIM_make_attr_uint64(sample->palette[j].R));
                SIM_attr_list_set_item(&res, i + 1, SIM_make_attr_uint64(sample->palette[j].G));
                SIM_attr_list_set_item(&res, i + 2, SIM_make_attr_uint64(sample->palette[j].B));
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
        } else {
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

bool
graph16_draw_sprite (graph16_t *core, graph16_sprite_t *sprite)
{
        int ret = 0;

        int spritew_on_screen = core->spritew;
        int spriteh_on_screen = core->spriteh;

        int sprite_size = 0;

        if (spritew_on_screen % 2 == 0) {

                sprite_size = spritew_on_screen / 2  * spriteh_on_screen;
        } else {

                SIM_log_spec_violation (1, graph16_to_conf(core), 0,
                                        "sprite's width is odd");
        }

        uint8 data[sprite_size];

        // TODO: read in chunks (because it is architecture rigth)
        ret = graph16_read_memory (core, PHYS_MEM, sprite->addr,
                                   sprite_size, data);
        if (ret == false) {
                SIM_LOG_ERROR(graph16_to_conf(core), 0,
                             "failed to read sprite from phys memory");
                return false;
        }


        // There is no wrapping; what is drawn offscreen stays offscreen, and is thrown away.
        if ((SCREEN_W - sprite->x) < spritew_on_screen)
                spritew_on_screen = SCREEN_W - sprite->x;

        if ((SCREEN_H - sprite->y) < spriteh_on_screen)
                spriteh_on_screen = SCREEN_H - sprite->y;

        int i = 0;
        for (i = 0; i < spriteh_on_screen; i++) {

                // Here we write sprite line by line. Memory is linear, so the offset is calculated
                // like a 3rd arg. Then we write line of sprite (we use spritew_on_screen, not core->spritew,
                // because of posible wrapping of screen). The 5th arg an offet in sprite data, here we
                // must use the real width of sprite (core->spritew)

                ret = graph16_write_memory (core, VIDEO_MEM, SCREEN_W / 2 * (sprite->y + i) + sprite->x / 2,
                                            spritew_on_screen / 2, data + i * core->spritew / 2);

                if (ret == false) {
                        SIM_LOG_ERROR(graph16_to_conf(core), 0,
                                     "failed to write sprite to video memory");
                        return false;
                }

        }

        return true;
}

bool
graph16_update_screen (graph16_t *core)
{
        int ret = 0;
        int screen_size = SCREEN_W * SCREEN_H;

        uint8 data[screen_size / 2]; // 2 pixels is coded by 1 byte

        ret = graph16_read_memory (core, VIDEO_MEM, 0x00, screen_size / 2, data);
        if (ret == false) {
                SIM_LOG_ERROR(graph16_to_conf(core), 0,
                             "failed to read from video memory");
                return false;
        }

        if (core->screen == NULL) {
                return true;            // There is no screen
        }

        uint32 *pixels = core->screen->pixels;
        if (pixels == NULL) {
                SIM_LOG_ERROR(graph16_to_conf(core), 0,
                             "pixels field in SDL_Surface screen is NULL");
                return false;
        }

        // Filling pixels' buffer
        SDL_LockSurface(core->screen);
        int i = 0;
        for (i = 0; i < screen_size / 2; i++) {

                uint32 color1 = SDL_MapRGB (core->screen->format,
                                            core->palette[(data[i] >> 4) & 0xF].R,
                                            core->palette[(data[i] >> 4) & 0xF].G,
                                            core->palette[(data[i] >> 4) & 0xF].B
                );

                uint32 color2 = SDL_MapRGB (core->screen->format,
                                            core->palette[(data[i]) & 0xF].R,
                                            core->palette[(data[i]) & 0xF].G,
                                            core->palette[(data[i]) & 0xF].B
                );

                pixels[i * 2]     = color1;
                pixels[i * 2 + 1] = color2;


                uint32 background = SDL_MapRGB (core->screen->format,
                                                core->palette[core->bg].R,
                                                core->palette[core->bg].G,
                                                core->palette[core->bg].B
                );
                if (data[i] != background) {
                        SIM_log_info (4, graph16_to_conf(core), 0, "X: %d, Y: %d, Color: %x\n",
                                                                   i * 2 % (SCREEN_W / 2),
                                                                   i / (SCREEN_W / 2),
                                                                   (data[i] >> 4) & 0xF);

                        SIM_log_info (4, graph16_to_conf(core), 0, "X: %d, Y: %d, Color: %x\n",
                                                                   (i * 2 + 1) % (SCREEN_W / 2),
                                                                   (i + 1) / (SCREEN_W / 2),
                                                                   (data[i] >> 4) & 0xF);
                }
        }
        SDL_UnlockSurface(core->screen);

        // Drawing on the screen pixels' buffer
        SDL_UpdateTexture(core->texture, NULL, pixels, SCREEN_W * sizeof (uint32)); // uint32 because we use ARGB format

        SDL_RenderClear(core->renderer);
        SDL_RenderCopy(core->renderer, core->texture, NULL, NULL);
        SDL_RenderPresent(core->renderer);

        return true;
}

static int
graph16_refresh_screen(void *arg)
{
        graph16_t *core = (graph16_t *)arg;
        SDL_Event event;
        ASSERT(core);

        /* TODO proper locking of core should be implemented */
        if (!core->renderer)
                return 0;
        while (core->refresh_active) {
                while (SDL_PollEvent(&event));
                SDL_Delay(1000);
        }

        SIM_log_info (4, graph16_to_conf(core), 0, "Screen refreshing thread: shutting down...\n");

        return 1;
}
