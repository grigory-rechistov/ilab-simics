#include <simics/device-api.h>
#include <simics/devs/io-memory.h>
#include "sample-interface.h"

#include "include/SDL2/SDL.h"

#define PAL_SIZE 16

#define NIBBLE_MASK     0xF
#define UCHAR_MASK      0xFF
#define BOOLEAN_MASK    0x1

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

        /* for memory */
        conf_object_t *phys_mem_obj;
        conf_object_t *video_mem_obj;


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

int delete_instance(conf_object_t *obj);

static void
simple_method(conf_object_t *obj, int arg);

static inline graph16_t *
conf_to_graph16(conf_object_t *obj)
{
        return SIM_object_data(obj);
}