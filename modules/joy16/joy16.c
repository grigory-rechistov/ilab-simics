/* joy16.c - starting point for a joystick of CHIP16

   This Software is part of Wind River Simics. The rights to copy, distribute,
   modify, or otherwise make use of this Software may be licensed only
   pursuant to the terms of an applicable Wind River license agreement.

   Copyright 2014-2015 Intel Corporation */

#include <simics/simulator-api.h> // For SIM_printf(), SIM_lookup_file()
#include <simics/device-api.h>
#include <simics/devs/io-memory.h>
#include <simics/devs/signal.h>

#include "sample-interface.h"
#include "include/SDL2/SDL.h"
#include "include/SDL2/SDL_thread.h"

#ifndef HOST_TYPE
#error HOST_TYPE must be defined when compiling this file: linux64, win64 etc
#endif

/* Use two level macro voodoo to convert HOST_TYPE to string
 See https://gcc.gnu.org/onlinedocs/cpp/Stringification.html for explanations */
#define XMACRO_STR(s) MACRO_STR(s)
#define MACRO_STR(s) #s
#define HOST_LIB_DIR XMACRO_STR(HOST_TYPE) "/lib"

#define BMP_NAME "images/joy16.bmp"
/* THREAD_SAFE_GLOBAL: joy16_bmp_path init */
static const char* joy16_bmp_path = NULL;

typedef struct {
        /* Simics configuration object */
        conf_object_t obj;

        /* device specific data */
        union joy16_reg
        {
        struct joy16_reg_map
                {
                    unsigned U     :1; // Up
                    unsigned D     :1; // Down
                    unsigned L     :1; // Left
                    unsigned R     :1; // Right
                    unsigned Slc   :1; // Select    <-- refers to --- Tab
                    unsigned St    :1; // Start     <-- refers to --- Return
                    unsigned A     :1; // A         <-- refers to --- F
                    unsigned B     :1; // B         <-- refers to --- G
                    unsigned empty :8;
                } map;
        uint16 byte;

        } value;

        /* SDL related data */
        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_Thread *refresh_thread;
        bool refresh_active;
} joy16_t;

/* Allocate memory for the object. */
static conf_object_t *
alloc_object(void *data)
{
        joy16_t *joy = MM_ZALLOC(1, joy16_t);
        return &joy->obj;
}

/* Filter events to leave only ones from keyboard */
static int joy16_event_filter(void* userdata, SDL_Event* event) {
        // TODO lock joystick state?
        joy16_t *joy = (joy16_t *)userdata;
        ASSERT(joy);
        ASSERT(joy->window);

        /* Leave only events we are interested in. the way we want.
        others are not supposed to get into the event queue
        !!! except user events. Thus use key_inject interface with caution */
        switch (event->type) {
            case SDL_KEYDOWN: return 1;
            case SDL_KEYUP:   return 1;
            default:          return 0;
        }
        return 0;
}
static int joy16_refresh_screen(void *arg) {
        joy16_t *joy = (joy16_t *)arg;
        ASSERT(joy);
        /* TODO proper locking of joy should be implemented; SDL_Atomic? */
        if (!joy->renderer) return 0;
        while (joy->refresh_active) {
                SDL_RenderPresent(joy->renderer);
                SDL_Delay(100);
        }
        // printf("Shutting down...\n");
        return 1;
}

lang_void *init_object(conf_object_t *obj, lang_void *data) {
        joy16_t *joy = (joy16_t *)obj;

        const char* name = SIM_object_name(obj);
        joy->window = SDL_CreateWindow (
        name,                              // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        128,                               // width, in pixels
        64,                                // height, in pixels
        SDL_WINDOW_SHOWN                   // flags 
        );

        if (joy->window == NULL) {
                SIM_LOG_INFO(1, obj, 0, "Failed to create SDL window");
                return obj;
        }
        joy->renderer = SDL_CreateRenderer(joy->window, -1, SDL_RENDERER_SOFTWARE);
        if (joy->renderer == NULL) {
                SIM_LOG_ERROR(obj, 0, "Failed to create SDL renderer");
                return obj;
        }
        SDL_SetRenderDrawColor(joy->renderer, 0xaa, 0xbb, 0xcc, 0xdd);
        SDL_RenderClear(joy->renderer);
        SDL_SetRenderDrawColor(joy->renderer, 0xff, 0, 0, 0);

        /* Load a nice picture */
        SDL_Surface *image = SDL_LoadBMP(joy16_bmp_path);
        if (image) {
                SDL_Texture *texture = SDL_CreateTextureFromSurface(joy->renderer, image);
                ASSERT(texture);
                SDL_RenderCopy(joy->renderer, texture, NULL, NULL);
                SDL_FreeSurface(image);
        } else {
                SIM_LOG_INFO(1, obj, 0, "Failed to load a BMP image");
                /* This is not critical though */
        }
        /* FIXME: a set of cleanup calls for texture should be in delete_instance() */

        SDL_RenderPresent(joy->renderer);

        // Filter events before entering the event queue
        SDL_SetEventFilter(joy16_event_filter, (void*)joy);

        /* Create a separate thread to refresh SDL GUI window */
        joy->refresh_active = true; /* A memory barrier would be nice here; SDL_Atomic? */
        joy->refresh_thread = SDL_CreateThread(joy16_refresh_screen, "joy16 refresh screen thread", (void *)joy);
        ASSERT(joy->refresh_thread);

        return obj;
}

int delete_instance(conf_object_t *obj) {
        /* NOTE: a session with GDB showed that this function is actually
           not called at all. Therefore, no cleanup is done, right?
           This has to be investigated eventually. */

        joy16_t *joy = (joy16_t *)obj;

        /* Shut down the GUI refresh thread */
        joy->refresh_active = false; // A poor man's synchronization method
        /* A memory barrier would be nice here; SDL_Atomic? */
        SIM_LOG_INFO(4, obj, 0, "Waiting for the refresh thread to return...");
        SDL_WaitThread(joy->refresh_thread, NULL);
        SDL_DestroyRenderer(joy->renderer);
        SDL_DestroyWindow(joy->window);
        return 1;
}

static exception_type_t
operation(conf_object_t *obj, generic_transaction_t *mop,
                 map_info_t info)
{
        joy16_t *joy = (joy16_t *)obj;
        unsigned offset = (SIM_get_mem_op_physical_address(mop)
                           + info.start - info.base);

        if (SIM_mem_op_is_read(mop)) {
                SIM_set_mem_op_value_le(mop, joy->value.byte);
                SIM_LOG_INFO(4, &joy->obj, 0, "read from offset %d: 0x%x",
                             offset, joy->value.byte);
        } else {
                SIM_LOG_SPEC_VIOLATION(1, &joy->obj, 0, "Write ignored %d %#llx",
                             offset, SIM_get_mem_op_value_le(mop));
        }
        return Sim_PE_No_Exception;
}

static set_error_t
set_value_attribute(void *arg, conf_object_t *obj,
                    attr_value_t *val, attr_value_t *idx)
{
        joy16_t *joy = (joy16_t *)obj;
        joy->value.byte = SIM_attr_integer(*val);
        SDL_RenderPresent(joy->renderer);
        return Sim_Set_Ok;
}

static attr_value_t
get_value_attribute(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        joy16_t *joy = (joy16_t *)obj;
        return SIM_make_attr_uint64(joy->value.byte);
}

static void
joy16_signal_raise(conf_object_t *obj) 
{
        joy16_t *joy = (joy16_t *)obj;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
                if (event.type == SDL_KEYDOWN) {
                        switch (event.key.keysym.sym) {
                        case SDLK_UP:
                                joy->value.map.U = 1;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keydown UP");
                                break;
                        case SDLK_DOWN:
                                joy->value.map.D = 1;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keydown DOWN");
                                break;
                        case SDLK_LEFT:
                                joy->value.map.L = 1;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keydown LEFT");
                                break;
                        case SDLK_RIGHT:
                                joy->value.map.R = 1;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keydown RIGHT");
                                break;
                        case SDLK_TAB:
                                joy->value.map.Slc = 1;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keydown SELECT");
                                break;
                        case SDLK_RETURN:
                                joy->value.map.St = 1;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keydown START");
                                break;
                        case SDLK_f:
                                joy->value.map.A = 1;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keydown A");
                                break;
                        case SDLK_g:
                                joy->value.map.B = 1;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keydown B");
                                break;
                        default:
                                SIM_LOG_INFO(4, &joy->obj, 0, "Other key pressed");
                                break;
                        }
                }
                else if (event.type == SDL_KEYUP) {
                        switch (event.key.keysym.sym) {
                        case SDLK_UP:
                                joy->value.map.U = 0;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keyup UP");
                                break;
                        case SDLK_DOWN:
                                joy->value.map.D = 0;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keyup DOWN");
                                break;
                        case SDLK_LEFT:
                                joy->value.map.L = 0;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keyup LEFT");
                                break;
                        case SDLK_RIGHT:
                                joy->value.map.R = 0;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keyup RIGHT");
                                break;
                        case SDLK_TAB:
                                joy->value.map.Slc = 0;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keyup SELECT");
                                break;
                        case SDLK_RETURN:
                                joy->value.map.St = 0;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keyup START");
                                break;
                        case SDLK_f:
                                joy->value.map.A = 0;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keyup A");
                                break;
                        case SDLK_g:
                                joy->value.map.B = 0;
                                SIM_LOG_INFO(4, &joy->obj, 0, "Keyup B");
                                break;
                        default:
                                SIM_LOG_INFO(4, &joy->obj, 0, "Other key released");
                                break;
                        }
                 }
                 else
                        ASSERT_MSG(0, "Wrong keyboard event type!");
        }
        if (joy->renderer) /* Update GUI window state */
                SDL_RenderPresent(joy->renderer);
}

static void
joy16_signal_lower(conf_object_t *obj) 
{
        // nothing to do
}

/* Inject a keyup/keydown event to the queue */
static void
joy16_key_inject (conf_object_t* obj, int arg)
{
        SDL_Event event = {0};

        /* because arg is signed int, the 31st bit is taken
           we need somehow to determine whether it is Keyup or Keydown
           thus we use 29th bit and after the condition we clear it */
        if(arg & (1 << 29)) {
            arg = arg ^ (1<<29);
            event.type = SDL_KEYDOWN;
            event.key.keysym.sym = arg & ~(1 << 31);
        }
        else {
            event.type = SDL_KEYUP;
            event.key.keysym.sym = arg & ~(1 << 31);
        }
        SDL_PushEvent(&event);
}

static set_error_t
set_add_log_attribute(void *arg, conf_object_t *obj, attr_value_t *val,
                      attr_value_t *idx)
{
        joy16_t *joy = (joy16_t *)obj;
        SIM_LOG_INFO(1, &joy->obj, 0, "%s", SIM_attr_string(*val));
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
                .class_desc = "Joystick of CHIP16",
                .description = "Joystick device of CHIP16 system."
        };
        conf_class_t *class = SIM_register_class("joy16", &funcs);

        /* Register VBLANK interrupt interface */
        static const signal_interface_t vblank = {
                .signal_raise = joy16_signal_raise,
                .signal_lower = joy16_signal_lower
        };
        SIM_register_interface(class, SIGNAL_INTERFACE, &vblank);

        /* Register the 'io_memory' interface, which is an example of a generic
           interface that is implemented by all memory mapped devices. */
        static const io_memory_interface_t memory_iface = {
                .operation = operation
        };
        SIM_register_interface(class, IO_MEMORY_INTERFACE, &memory_iface);

        /* Register the 'key_inject' interface for user keyboard events */
        static const sample_interface_t key_inject = {
                .simple_method = joy16_key_inject
        };
        SIM_register_interface(class, SAMPLE_INTERFACE, &key_inject);

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

        if (SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0) {
                int res = 0;
                SIM_printf("SDL video/events haven't been initialized, doing it now\n");
                res = SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
                if (res != 0) SIM_printf("Warning: SDL_INIT_SUBSYTEM returned not zero\n");
        }

        /* We will need to know a path to a BMP file with joy16 bitmap.
           Add a directory with it to Simics directory list, and do the lookup.
           FIXME not sure if this will work for a packaged installation, not locally built one.
          */
        SIM_add_directory(HOST_LIB_DIR, false);
        joy16_bmp_path = SIM_lookup_file(BMP_NAME); /* NOTE: returned memory won't be freed anywhere */
        SIM_clear_exception(); /* Sometimes this BMP may not be found */
}

#undef XMACRO_STR
#undef MACRO_STR
