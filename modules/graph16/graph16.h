#include "sample-interface.h"

#include <simics/device-api.h>
#include <simics/devs/io-memory.h>
#include <simics/devs/memory-space.h>
#include <simics/model-iface/memory-page.h>

#include "include/SDL2/SDL.h"
#include "include/SDL2/SDL_thread.h"

#define SCREEN_W 320
#define SCREEN_H 240

#define PAL_SIZE 16

#define PHYS_MEM  0
#define VIDEO_MEM 1

typedef struct {
        uint8 opcode;
        uint8 length;

} graph16_instr_t;

typedef struct {
        uint16 x;
        uint16 y;
        uint16 addr;

} graph16_sprite_t;

typedef struct {
        uint8 R;
        uint8 G;
        uint8 B;

} graph16_pal_item;

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

        /* memory */
        conf_object_t *physical_mem_obj;
        conf_object_t *video_mem_obj;

        /* physical memory interfaces */
        const memory_space_interface_t *physical_mem_space_iface;
        const memory_page_interface_t *physical_mem_page_iface;
        const breakpoint_trigger_interface_t *physical_mem_bp_trig_iface;

        /* video memory interfaces */
        const memory_space_interface_t *video_mem_space_iface;
        const memory_page_interface_t *video_mem_page_iface;
        const breakpoint_trigger_interface_t *video_mem_bp_trig_iface;

        /* device specific data */
        unsigned value;

        /* registers */
        uint8   bg;             // (Nibble) Color index of background layer
        uint8   spritew;        // (Unsigned byte) Width of sprite(s) to draw
        uint8   spriteh;        // (Unsigned byte) Height of sprite(s) to draw
        bool    hflip;          // (Boolean) Flip sprite(s) to draw, horizontally
        bool    vflip;          // (Boolean) Flip sprite(s) to draw, vertically

        graph16_pal_item palette[PAL_SIZE];


        graph16_instr_t instruction;

        uint32 temp[24];

        // SDL objects
        SDL_Window *window;

        SDL_Surface  *screen;
        SDL_Texture  *texture;
        SDL_Renderer *renderer;

        SDL_Thread *refresh_thread;
        bool refresh_active;

} graph16_t;

int delete_instance(conf_object_t *obj);

static void simple_method(conf_object_t *obj, int arg);

int graph16_draw_sprite (graph16_t *core, graph16_sprite_t *sprite);

int graph16_update_screen (graph16_t *core);

static inline graph16_t * conf_to_graph16(conf_object_t *obj)
{
        return SIM_object_data(obj);
}

static inline conf_object_t * graph16_to_conf(graph16_t *sr)
{
        return &sr->obj;
}

static generic_transaction_t create_generic_transaction (conf_object_t *initiator, mem_op_type_t type,
                           physical_address_t phys_address,
                           physical_address_t len, uint8 *data,
                           endianness_t endian);

bool graph16_write_memory (graph16_t *core, int mem_switch, physical_address_t phys_address,
                    physical_address_t len, uint8 *data);

bool graph16_read_memory (graph16_t *core, int mem_switch, physical_address_t phys_address,
                   physical_address_t len, uint8 *data);

static int graph16_refresh_screen(void *arg);
