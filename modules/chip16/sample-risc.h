/*
  sample-risc.h - sample code for a stand-alone RISC processor

  This Software is part of Wind River Simics. The rights to copy, distribute,
  modify, or otherwise make use of this Software may be licensed only
  pursuant to the terms of an applicable Wind River license agreement.
  
  Copyright 2010-2014 Intel Corporation

*/

#ifndef SAMPLE_RISC_H
#define SAMPLE_RISC_H

#include <simics/base/types.h>
#include <simics/base/log.h>
#include <simics/base/bigtime.h>
#include <simics/processor/types.h>
#include <simics/model-iface/execute.h>
#include <simics/devs/frequency.h>
#include <simics/processor-api.h>
#include <simics/devs/memory-space.h>
#include <simics/model-iface/breakpoints.h>
#include <simics/model-iface/memory-page.h>
#include "event-queue.h"
#include "sample-risc-exec.h"


#include "event-queue-types.h"
#include "sample-risc-exec.h"

struct sample_risc;
typedef struct sample_risc sample_risc_t;

typedef short sample_reg_number_t;
typedef uint64 (*reg_get_function_ptr)(sample_risc_t *core, int n);
typedef void (*reg_set_function_ptr)(sample_risc_t *core, int n, uint64 value);


/* get the current program counter for the core */
logical_address_t chip16_get_pc(sample_risc_t *core);

/* set the current program counter for the core */
void chip16_set_pc(sample_risc_t *core, logical_address_t value);

/* convert a logical address to physical address */
int chip16_logical_to_physical(sample_risc_t *core,
                                    logical_address_t la_addr,
                                    access_t access_type,
                                    physical_address_t *pa_addr);

/* disassemble a string into (a) a boolean success code,
   and (b) a string (if successful) */
tuple_int_string_t chip16_disassemble(conf_object_t *obj,
                                           generic_address_t address,
                                           attr_value_t instruction_data,
                                           int sub_operation);

void chip16_fetch_and_execute_instruction(sample_risc_t *core);

/*
 * Services provided to the processor
 */
/* to declare a register */
/* registers have to be declared for each core */
int chip16_add_register_declaration(sample_risc_t *core,
                                         const char *name,
                                         int reg_number,
                                         reg_get_function_ptr get,
                                         reg_set_function_ptr set,
                                         int catchable);

/* check a virtual breakpoint */
void chip16_check_virtual_breakpoints(sample_risc_t *core,
                                           access_t access,
                                           logical_address_t virt_start,
                                           generic_address_t len,
                                           uint8 *data);

/* fetch an instruction from memory */
bool chip16_fetch_instruction(sample_risc_t *core,
                                   physical_address_t pa,
                                   physical_address_t len,
                                   uint8 *data,
                                   int check_bp);

/* read and write generally */
bool chip16_write_memory(sample_risc_t *core,
                              physical_address_t pa,
                              physical_address_t len,
                              uint8 *data,
                              int check_bp);

bool chip16_read_memory(sample_risc_t *core,
                             physical_address_t pa,
                             physical_address_t len,
                             uint8 *data,
                             int check_bp);

void chip16_release_and_share(sample_risc_t *core,
                                   physical_address_t pa);

/* get the running/stopped state of the core */
execute_state_t chip16_state(sample_risc_t *core);

/* increment the number of steps, cycles, or check the next event */
void chip16_increment_cycles(sample_risc_t *core, int n);
void chip16_increment_steps(sample_risc_t *core, int n);
int  chip16_next_cycle_event(sample_risc_t *core);
int  chip16_next_step_event(sample_risc_t *core);

typedef struct {
        conf_object_t *object;
        const frequency_listener_interface_t *iface;
        char *port_name;
} frequency_target_t;

typedef VECT(frequency_target_t) frequency_target_list_t;

/* Registers are handled by creating (at initialization time) a
 * table of registers.  Each table entry has a string name, and
 * an internal register number (in case we have an array of
 * registers, like the gpr or fpr or vmx or ...), as well as two
 * functions -- one to get the register value and the other to
 * set the register value.
 *
 * The Simics register name is the "name"; the Simics register
 * number is the index into the table of registers.  We implement
 * the register table with the Simics Vector type.
 */
typedef struct register_description {
        const char          *name;
        int                 reg_number;
        reg_get_function_ptr   get_reg_value;
        reg_set_function_ptr   set_reg_value;
        int                 catchable;
} register_description_t;


typedef VECT(register_description_t) register_table;

/*
 * The main sample_risc class
 */
typedef struct sample_risc {
        /* pointer to the corresponding Simics object */
        conf_object_t *obj;

        /* Pointer to cell */
        conf_object_t *cell;
        const cell_inspection_interface_t *cell_iface;

        conf_object_t *current_context;
        const breakpoint_query_interface_t *context_bp_query_iface;
        const breakpoint_trigger_interface_t *context_bp_trig_iface;

        conf_object_t *phys_mem_obj;
        const memory_space_interface_t *phys_mem_space_iface;
        const memory_page_interface_t *phys_mem_page_iface;
        const breakpoint_trigger_interface_t *phys_mem_bp_trig_iface;

        int thread_id;
        int enabled;
        double freq_mhz;

        cycles_t idle_cycles;

        logical_address_t toy_pc;
        uint32 toy_msr;
        uint32 toy_reg[16];
        double toy_freq_mhz;


        /* The list of registers for this class of cores */
        register_table reg_table;


        /* page cache */
        int number_of_cached_pages;
        VECT(mem_page_t) cached_pages;

        /* execute state */
        execute_state_t state;

        /* cycle queue */
        cycles_t current_cycle;
        event_queue_t cycle_queue;

        /* step queue */
        pc_step_t current_step;
        event_queue_t step_queue;

        /* the offset that has to be added to the frequency-rescaled
           cycle counter to produce the absolute time */
        bigtime_t time_offset;

        /* frequency related data */
        uint64 freq_hz;
        conf_object_t *frequency_dispatcher;
        char *frequency_dispatcher_port;
        const simple_dispatcher_interface_t *frequency_dispatcher_iface;
        frequency_target_list_t frequency_targets;
} sample_risc_t;

static inline conf_object_t *
sr_to_conf_obj(sample_risc_t *sr)
{
        return sr->obj;
}

static inline sample_risc_t *
conf_obj_to_sr(conf_object_t *obj)
{
        return SIM_object_data(obj);
}

void sample_risc_cycle_event_posted(sample_risc_t *sr);
void sample_risc_step_event_posted(sample_risc_t *sr);

static inline conf_object_t *
sr_core_to_conf_obj(sample_risc_t *sr)
{
        return sr->obj;
}

static inline sample_risc_t *
conf_obj_to_sr_core(conf_object_t *obj)
{
        return SIM_object_data(obj);
}


#endif
