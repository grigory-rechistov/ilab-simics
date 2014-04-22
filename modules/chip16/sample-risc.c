/*
  sample-risc.c - sample code for a stand-alone RISC processor

  This Software is part of Wind River Simics. The rights to copy, distribute,
  modify, or otherwise make use of this Software may be licensed only
  pursuant to the terms of an applicable Wind River license agreement.
  
  Copyright 2010-2014 Intel Corporation

*/

#include "sample-risc.h"
#include <simics/processor-api.h>
#include <simics/util/strbuf.h>
#include <simics/util/swabber.h>

#include <simics/model-iface/exception.h>
#include <simics/model-iface/int-register.h>
#include <simics/model-iface/processor-info.h>

// TODO: should probably move to Processor API
#include <simics/simulator-iface/context-tracker.h>

#include <simics/devs/signal.h>

#include "event-queue.h"
#include "sample-risc.h"
#include "sample-risc-memory.h"
#include "sample-risc-queue.h"

// TODO: should not be needed, violates the Processor API
#include <simics/simulator/hap-consumer.h>

#include "event-queue.h"
#include "sample-risc-memory.h"
#include "sample-risc-exec.h"
#include "sample-risc-cycle.h"
#include "sample-risc-step.h"
#include "sample-risc-frequency.h"

/* fixed size instructions of 4 bytes */
#define INSTR_SIZE 4


/*
 *           31   28 27   24 23   20 19   16 15   12 11    8 7     4 3     0
 *          |-------|-------|-------|-------|-------|-------|-------|-------|
 *
 *
 *   nop    |   0   |                ---                                    |
 *
 *   branch |   1   |  mode |
 *                      imm |  ---  |  ---  |   imm_value                   |
 *                      ind |  src  |            ---                        |
 *
 *   load   |   2   |  mode |
 *                     imm  |  ---  |  dst  |  (imm_value)                  |
 *                     ind  | (src) |  dst  |   ---                         |
 *
 *   store  |   3   |  mode |
 *                     imm  |  src  |  ---  |  (imm_value)                  |
 *                     ind  |  src  | (dst) |   ---                         |
 *
 *   idle   |   4   |      ---              |  imm_value                    |
 *
 *   magic  |   5   |                ---                                    |
 *
 *   add    |   6   |  ---- |  src  |  dst  |  imm_value                    |
 *
 */


#define INSTR_OP(i) (((i) >> 28) & 0xf)
#define INSTR_MODE(i) (((i) >> 24) & 0xf)
#define INSTR_SRC_REG(i) (((i) >> 20) & 0xf)
#define INSTR_DST_REG(i) (((i) >> 16) & 0xf)
#define INSTR_IMMEDIATE(i) ((i) & 0xffff)

typedef enum {
        Instr_Op_Nop    = 0,
        Instr_Op_Branch = 1,
        Instr_Op_Load   = 2,
        Instr_Op_Store  = 3,
        Instr_Op_Idle   = 4,
        Instr_Op_Magic  = 5,
        Instr_Op_Add    = 6
} instr_op_t;

typedef enum {
        Instr_Mode_Immediate = 0,
        Instr_Mode_Indirect  = 1
} instr_mode_t;


/* THREAD_SAFE_GLOBAL: hap_Control_Register_Read init */
hap_type_t hap_Control_Register_Read;
/* THREAD_SAFE_GLOBAL: hap_Control_Register_Write init */
hap_type_t hap_Control_Register_Write;
/* THREAD_SAFE_GLOBAL: hap_Magic_Instruction init */
hap_type_t hap_Magic_Instruction;

/*
 * keep in sync with toy_init_registers
 */
typedef enum {
        Reg_Idx_R0,
        Reg_Idx_R1,
        Reg_Idx_R2,
        Reg_Idx_R3,
        Reg_Idx_R4,
        Reg_Idx_R5,
        Reg_Idx_R6,
        Reg_Idx_R7,
        Reg_Idx_R8,
        Reg_Idx_R9,
        Reg_Idx_R10,
        Reg_Idx_R11,
        Reg_Idx_R12,
        Reg_Idx_R13,
        Reg_Idx_R14,
        Reg_Idx_R15,
        Reg_Idx_PC,
        Reg_Idx_Msr,
} register_index_t;

static sample_risc_t *
associated_toy(sample_risc_t *core) // FIXME  remove
{
        return core;
}

static attr_value_t
toy_get_registers(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        sample_risc_t *toy = associated_toy(core);
        attr_value_t ret = SIM_alloc_attr_list(16 + 2);

        /*
         * keep in sync with toy_init_registers
         */
        for (unsigned i = 0; i < 16; i++)
                SIM_attr_list_set_item(
                        &ret, i, SIM_make_attr_uint64(toy->toy_reg[i]));

        SIM_attr_list_set_item(&ret, 16, SIM_make_attr_uint64(toy->toy_pc));
        SIM_attr_list_set_item(&ret, 17, SIM_make_attr_uint64(toy->toy_msr));
        return ret;
}

static set_error_t
toy_set_registers(void *arg, conf_object_t *obj,
                  attr_value_t *val, attr_value_t *idx)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        sample_risc_t *toy = associated_toy(core);

        /*
         * keep in sync with toy_init_registers
         */
        if (SIM_attr_list_size(*val) < 18)
                return Sim_Set_Illegal_Value;
        for (unsigned i = 0; i < 16; i++)
                toy->toy_reg[i] =
                        SIM_attr_integer(SIM_attr_list_item(*val, i));
        toy->toy_pc = SIM_attr_integer(SIM_attr_list_item(*val, 16));
        toy->toy_msr = SIM_attr_integer(SIM_attr_list_item(*val, 17));
        return Sim_Set_Ok;
}

static attr_value_t
toy_get_freq_mhz(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        sample_risc_t *toy = associated_toy(core);
        return SIM_make_attr_floating(toy->toy_freq_mhz);
}

static set_error_t
toy_set_freq_mhz(void *arg, conf_object_t *obj,
                 attr_value_t *val, attr_value_t *idx)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        sample_risc_t *toy = associated_toy(core);
        toy->toy_freq_mhz = SIM_attr_floating(*val);
        return Sim_Set_Ok;
}

logical_address_t
toy_get_pc(sample_risc_t *core)
{
        sample_risc_t *toy = associated_toy(core);
        return toy->toy_pc;
}

void
toy_set_pc(sample_risc_t *core, logical_address_t pc)
{
        sample_risc_t *toy = associated_toy(core);
        toy->toy_pc = pc;
}


uint32
toy_get_msr(sample_risc_t *core)
{
        sample_risc_t *toy = associated_toy(core);
        return toy->toy_msr;
}

void
toy_set_msr(sample_risc_t *core, uint32 msr)
{
        sample_risc_t *toy = associated_toy(core);
        toy->toy_msr = msr;
}

/* get pc for the core */
uint64
toy_read_pc(sample_risc_t *core, int i)
{
        return toy_get_pc(core);
}

/* set pc for the core */
void
toy_write_pc(sample_risc_t *core, int i, uint64 value)
{
        toy_set_pc(core,value);
}


/* get msr for the core */
uint64
toy_read_msr(sample_risc_t *core, int i)
{
        SIM_c_hap_occurred_always(hap_Control_Register_Read,
                                  core->obj,
                                  Reg_Idx_Msr,
                                  Reg_Idx_Msr);
        return toy_get_msr(core);
}

/* set msr for the core */
void
toy_write_msr(sample_risc_t *core, int i, uint64 value)
{
        SIM_c_hap_occurred_always(hap_Control_Register_Write,
                                  core->obj,
                                  Reg_Idx_Msr,
                                  value);
        toy_set_msr(core,value);
}


/* get gpr[i] for the core */
uint64
toy_get_gpr(sample_risc_t *core, int i)
{
        sample_risc_t *toy = associated_toy(core);
        return toy->toy_reg[i];
}

/* set gpr[i] for the core */
void
toy_set_gpr(sample_risc_t *core, int i, uint64 value)
{
        sample_risc_t *toy = associated_toy(core);
        toy->toy_reg[i] = value;
}

void
toy_init_registers(void *crc)
{
        /*
         * keep in sync with toy_set/get_registers
         */
}


/* access_type is Sim_Access_Read, Sim_Access_Write, Sim_Access_Execute */
/* returns boolean */
int
toy_logical_to_physical(sample_risc_t *core, logical_address_t ea,
                        physical_address_t *pa, access_t access_type)
{
        /* no address mapping for the toy system */
        *pa = ea;
        return 1;
}

uint32
sample_core_read_memory32(sample_risc_t *core, logical_address_t la,
                          physical_address_t pa)
{
        uint32 data = 0;
        sample_core_check_virtual_breakpoints(
                core, Sim_Access_Read, la, 4, (uint8 *)(&data));
        sample_core_read_memory(core, pa, 4, (uint8 *)&data, 1);
        return UNALIGNED_LOAD_BE32(&data);
}

void
sample_core_write_memory32(sample_risc_t *core, logical_address_t la,
                           physical_address_t pa, uint32 value)
{
        uint32 data;
        UNALIGNED_STORE_BE32(&data, value);
        sample_core_check_virtual_breakpoints(
                core, Sim_Access_Write, la, 4, (uint8 *)(&data));
        sample_core_write_memory(core, pa, 4, (uint8 *)&data, 1);

        /* Hint to the simulator that a page is likely to be
           sharable. The heuristic here is that a write to the last
           location on the page may be from a memset, and that's a
           good time to share the page. This works well for simple
           linear access patterns. */
        if ((pa & (4096-1)) == 4092)
                sample_core_release_and_share(core, pa);
}

char *
toy_string_decode(sample_risc_t *core, uint32 instr)
{
        strbuf_t b = SB_INIT;
        switch (INSTR_OP(instr)) {
        case Instr_Op_Nop:
                return MM_STRDUP("nop");

        case Instr_Op_Branch:
                sb_addstr(&b, "branch");
                if (INSTR_MODE(instr) == Instr_Mode_Immediate)
                        sb_addfmt(&b, " 0x%x", INSTR_IMMEDIATE(instr));
                else
                        sb_addfmt(&b, " r%d", INSTR_SRC_REG(instr));
                return sb_detach(&b);

        case Instr_Op_Load:
                sb_addstr(&b, "load");
                if (INSTR_MODE(instr) == Instr_Mode_Immediate)
                        sb_addfmt(&b, " (0x%x) -> r%d",
                                  INSTR_IMMEDIATE(instr), INSTR_DST_REG(instr));
                else
                        sb_addfmt(&b, " (r%d) -> r%d",
                                  INSTR_SRC_REG(instr), INSTR_DST_REG(instr));
                return sb_detach(&b);

        case Instr_Op_Store:
                sb_addstr(&b, "store");
                if (INSTR_MODE(instr) == Instr_Mode_Immediate)
                        sb_addfmt(&b, " r%d -> (0x%x)",
                                  INSTR_SRC_REG(instr), INSTR_IMMEDIATE(instr));
                else
                        sb_addfmt(&b, " r%d -> (r%d)",
                                  INSTR_SRC_REG(instr), INSTR_DST_REG(instr));
                return sb_detach(&b);

        case Instr_Op_Idle:
                sb_addfmt(&b, "idle 0x%x", INSTR_IMMEDIATE(instr));
                return sb_detach(&b);

        case Instr_Op_Magic:
                return MM_STRDUP("magic");

        case Instr_Op_Add:
                sb_addfmt(&b, "add r%d + r%d + 0x%x -> r%d",
                          INSTR_SRC_REG(instr), INSTR_DST_REG(instr),
                          INSTR_IMMEDIATE(instr), INSTR_DST_REG(instr));
                return sb_detach(&b);

        default:
                SIM_LOG_ERROR(core->obj, 0, "unknown instruction");
                return MM_STRDUP("unknown");
        }
}


#define INCREMENT_PC(core) toy_set_pc(core, toy_get_pc(core)+ INSTR_SIZE)

void
toy_execute(sample_risc_t *core, uint32 instr)
{
        cycles_t stall_cycles = 0;
        cycles_t cycles_left = 0;

        /* execute instruction */
        switch (INSTR_OP(instr)) {

        case Instr_Op_Nop:
                sample_core_increment_cycles(core, 1);
                sample_core_increment_steps(core, 1);
                INCREMENT_PC(core);
                break;

        case Instr_Op_Branch:
                sample_core_increment_cycles(core, 1);
                sample_core_increment_steps(core, 1);
                INCREMENT_PC(core);
                switch (INSTR_MODE(instr)) {
                case Instr_Mode_Immediate:
                        toy_set_pc(core, INSTR_IMMEDIATE(instr));
                        break;
                case Instr_Mode_Indirect:
                        toy_set_pc(core,
                                   toy_get_gpr(core, INSTR_SRC_REG(instr)));
                        break;
                default:
                        SIM_LOG_ERROR(core->obj, 0,
                                      "unknown instruction mode");
                }

                break;

        case Instr_Op_Load:
                sample_core_increment_cycles(core, 1);
                sample_core_increment_steps(core, 1);
                INCREMENT_PC(core);
                switch (INSTR_MODE(instr)) {
                case Instr_Mode_Immediate: {
                        logical_address_t la = INSTR_IMMEDIATE(instr);
                        physical_address_t pa;
                        bool ok = toy_logical_to_physical(core, la, &pa,
                                                          Sim_Access_Read);
                        if (ok) {
                                uint32 value = sample_core_read_memory32(
                                        core, la, pa);
                                toy_set_gpr(core, INSTR_DST_REG(instr), value);
                        }
                        break;
                }

                case Instr_Mode_Indirect: {
                        logical_address_t la =
                                toy_get_gpr(core, INSTR_SRC_REG(instr));
                        physical_address_t pa;
                        bool ok = toy_logical_to_physical(core, la, &pa,
                                                          Sim_Access_Read);
                        if (ok) {
                                uint32 value = sample_core_read_memory32(
                                        core, la, pa);
                                toy_set_gpr(core, INSTR_DST_REG(instr), value);
                        }
                        break;
                }

                default:
                        SIM_LOG_ERROR(core->obj, 0,
                                      "unknown instruction mode");
                        break;
                }
                break;

        case Instr_Op_Store:
                sample_core_increment_cycles(core, 1);
                sample_core_increment_steps(core, 1);
                INCREMENT_PC(core);
                switch (INSTR_MODE(instr)) {
                case Instr_Mode_Immediate: {
                        logical_address_t la = INSTR_IMMEDIATE(instr);
                        physical_address_t pa;
                        bool ok = toy_logical_to_physical(core, la, &pa,
                                                          Sim_Access_Write);
                        if (ok) {
                                uint32 value = toy_get_gpr(core,
                                                         INSTR_SRC_REG(instr));
                                sample_core_write_memory32(core, la, pa,
                                                           value);
                        }
                        break;
                }

                case Instr_Mode_Indirect: {
                        logical_address_t la =
                                toy_get_gpr(core, INSTR_DST_REG(instr));
                        physical_address_t pa;
                        bool ok = toy_logical_to_physical(core, la, &pa,
                                                          Sim_Access_Write);
                        if (ok) {
                                uint32 value = toy_get_gpr(core,
                                                         INSTR_SRC_REG(instr));
                                sample_core_write_memory32(core, la, pa,
                                                           value);
                        }
                        break;
                }

                default:
                        SIM_LOG_ERROR(core->obj, 0,
                                      "unknown instruction mode");
                        break;
                }
                break;

        case Instr_Op_Idle:
                if (core->idle_cycles) {
                        stall_cycles = core->idle_cycles;
                } else {
                        stall_cycles = INSTR_IMMEDIATE(instr) + 1;
                }
                cycles_left = sample_core_next_cycle_event(core);
                if (stall_cycles > cycles_left) {
                        core->idle_cycles = stall_cycles - cycles_left;
                        sample_core_increment_cycles(core, cycles_left);
                } else {
                        core->idle_cycles = 0;
                        sample_core_increment_cycles(core, stall_cycles);
                        sample_core_increment_steps(core, 1);
                        INCREMENT_PC(core);
                }
                break;

        case Instr_Op_Magic:
                sample_core_increment_cycles(core, 1);
                sample_core_increment_steps(core, 1);
                INCREMENT_PC(core);
                SIM_c_hap_occurred(hap_Magic_Instruction,
                                   core->obj,
                                   (unsigned long)0,
                                   (int64)0);
                break;

        case Instr_Op_Add:
                sample_core_increment_cycles(core, 1);
                sample_core_increment_steps(core, 1);
                INCREMENT_PC(core);
                toy_set_gpr(core, INSTR_DST_REG(instr),
                            (toy_get_gpr(core, INSTR_SRC_REG(instr))
                             + toy_get_gpr(core, INSTR_DST_REG(instr))
                             + INSTR_IMMEDIATE(instr)));
                break;

        default:
                SIM_LOG_ERROR(core->obj, 0,
                              "unknown instruction");
                break;
        }
}


void
toy_fetch_and_execute_instruction(sample_risc_t *core)
{
        /* get instruction */
        logical_address_t pc = toy_get_pc(core);
        physical_address_t pa;

        int ok = toy_logical_to_physical(core, pc, &pa, Sim_Access_Execute);
        if (ok) {
                uint32 instr;
                sample_core_fetch_instruction(core, pa, INSTR_SIZE,
                                              (uint8 *)(&instr), true);
                sample_core_check_virtual_breakpoints(
                        core, Sim_Access_Execute, pc, 4, (uint8 *)(&instr));
                instr = UNALIGNED_LOAD_BE32(&instr);
                if (sample_core_state(core) != State_Stopped) {
                        /* breakpoint triggered - execute instruction */
                        toy_execute(core, instr);
                }
        }
}


static inline sample_risc_t * // FIXME remove
associated_queue(sample_risc_t *core)
{
        return core;
}

static sample_risc_t *
associated_memory(sample_risc_t *core) // FIXME remove
{
        return core;
}


static register_table *
associated_register_table(sample_risc_t *core)
{
        register_table *rt = &(core->reg_table);
        return rt;
}

/*
 * physical_memory_space attribute functions
 */
static attr_value_t
sample_core_get_physical_memory(void *arg, conf_object_t *obj,
                                attr_value_t *idx)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        return SIM_make_attr_object(core->phys_mem_obj);
}

static set_error_t
sample_core_set_physical_memory(void *arg, conf_object_t *obj,
                                attr_value_t *val, attr_value_t *idx)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
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

        core->phys_mem_obj = oval;
        core->phys_mem_space_iface = mem_space_iface;
        core->phys_mem_page_iface = mem_page_iface;
        core->phys_mem_bp_trig_iface = bp_trig_iface;

        return Sim_Set_Ok;
}

/*
 * core_enabled attribute functions
 */
static attr_value_t
sample_core_get_enabled(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        return SIM_make_attr_boolean(core->enabled);
}


static set_error_t
sample_core_set_enabled(void *arg, conf_object_t *obj,
                        attr_value_t *val, attr_value_t *idx)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        core->enabled = SIM_attr_boolean(*val);
        return Sim_Set_Ok;
}

/*
 * idle_cycles attribute functions
 */
static attr_value_t
get_idle_cycles(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        return SIM_make_attr_uint64(core->idle_cycles);
}

static set_error_t
set_idle_cycles(void *arg, conf_object_t *obj,
                attr_value_t *val, attr_value_t *idx)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        core->idle_cycles = SIM_attr_integer(*val);
        return Sim_Set_Ok;
}

/*
 * context_handler interface functions
 */
static conf_object_t *
get_current_context(conf_object_t *obj)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        return core->current_context;
}

static int
set_current_context(conf_object_t *obj, conf_object_t *ctx)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        core->current_context = ctx;
        core->context_bp_query_iface = SIM_c_get_interface(
                core->current_context, BREAKPOINT_QUERY_INTERFACE);
        core->context_bp_trig_iface = SIM_c_get_interface(
                core->current_context, BREAKPOINT_TRIGGER_INTERFACE);
        ASSERT(core->context_bp_query_iface && core->context_bp_trig_iface);
        return 1;
}

/*
 * processor_info_v2 interface functions
 */
tuple_int_string_t
sample_core_disassemble(conf_object_t *obj, generic_address_t address,
                        attr_value_t instruction_data, int sub_operation)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);

        SIM_LOG_INFO(2, core->obj, 0,
                     "disassembling instruction at address 0x%llx", address);
        if (sub_operation != 0 && sub_operation != -1)
                return (tuple_int_string_t){0, NULL};

        if (SIM_attr_data_size(instruction_data) < 4) {
                /* The opcode is not complete. Tell the caller that we need 4
                   bytes. */
                return (tuple_int_string_t){-4, NULL};
        }

        uint32 instr = UNALIGNED_LOAD_BE32(SIM_attr_data(instruction_data));
        return (tuple_int_string_t){4, toy_string_decode(core, instr)};
}

static void
sample_core_set_program_counter(conf_object_t *obj,
                                logical_address_t pc)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        SIM_LOG_INFO(2, core->obj, 0,
                     "setting program counter to 0x%llx", pc);
        toy_set_pc(core, pc);
}

static logical_address_t
sample_core_get_program_counter(conf_object_t *obj)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        return sample_core_get_pc(core);
}


static physical_block_t
sample_core_logical_to_physical_block(conf_object_t *obj,
                                      logical_address_t address,
                                      access_t access_type)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        physical_block_t ret;
        ret.valid = sample_core_logical_to_physical(core, address, access_type,
                                                    &ret.address);
        ret.block_start = ret.address;
        ret.block_end = ret.address;
        SIM_LOG_INFO(2, core->obj, 0,
                     "logical address 0x%llx to physical address 0x%llx",
                     address, ret.address);
        return ret;
}

static processor_mode_t
sample_core_get_processor_mode(conf_object_t *obj)
{
        return Sim_CPU_Mode_User;  /* we don't have different modes */
}

static int
sample_core_enable_processor(conf_object_t *obj)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        if (core->enabled)
                return 1;
        core->enabled = 1;
        return 0;
}

static int
sample_core_disable_processor(conf_object_t *obj)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        if (!core->enabled)
                return 1;
        core->enabled = 0;
        return 0;
}

static int
sample_core_iface_get_enabled(conf_object_t *obj)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        return core->enabled;
}

static cpu_endian_t
sample_core_get_endian(conf_object_t *obj)
{
        return Sim_Endian_Big;
}

static conf_object_t *
sample_core_get_physical_memory_iface(conf_object_t *obj)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        return core->phys_mem_obj;
}

static int
sample_core_get_logical_address_width(conf_object_t *obj)
{
        return 64;
}

static int
sample_core_get_physical_address_width(conf_object_t *obj)
{
        return 64;
}

static const char *
sample_core_architecture(conf_object_t *obj)
{
        return "sample";
}

/*
 * registers
 */

/* Registers are handled by creating (at initialization time) a
 * table of registers.  Each table entry has a string name, and
 * an internal register number (in case we have an array of
 * registers, like the gpr or fpr or vmx or ...), as well as two
 * functions -- one to get the register value and the other to
 * set the register value.
 *
 * Registers are defined for each core class -- not each core.
 * We assume all cores have the same (set of) registers.  Different
 * instances, so different values; but the same set of registers.
 *
 * The Simics register name is the "name"; the Simics register
 * number is the index into the table of registers.  We implement
 * the register table with the Simics Vector type.
 */

static void
init_register_table(sample_risc_t *crc)
{
        register_table *rt = &(crc->reg_table);
        VINIT(*rt);
}


int
search_register_table(sample_risc_t *crc, const char *name)
{
        register_table *rt = &(crc->reg_table);
        VFORI(*rt, i) {
                register_description_t *rd;
                rd = &VGET(*rt, i);
                if (strcmp(name, rd->name) == 0)
                        return i;
        }

        /* not found in table, return illegal index */
        return -1;
}

int
sample_core_add_register_declaration(sample_risc_t *core,
                                     const char *name, int reg_number,
                                     reg_get_function_ptr get,
                                     reg_set_function_ptr set,
                                     int catchable)
{
        register_table *rt = &(core->reg_table);
        register_description_t *rd;

        /* we wish to define a register "name" */
        /* first see if we already have one */
        int i = search_register_table(core, name);

        if (i != -1) {
                rd = &VGET(*rt, i);
        } else {
                /* A new register -- make sure we have space */
                i = VLEN(*rt);
                VRESIZE(*rt, i + 1);
                rd = &VGET(*rt, i);
                /* and remember the name */
                rd->name = MM_STRDUP(name);
        }

        /* now that we have a register, define its properties.
         * at the moment, this is just its read/write functions */
        rd->reg_number = reg_number;
        rd->get_reg_value = get;
        rd->set_reg_value = set;
        rd->catchable = catchable;

        return i;
}

static int
sample_core_get_register_number(conf_object_t *obj, const char *name)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        int i = search_register_table(core, name);

        if (i == -1) {
                SIM_LOG_ERROR(core->obj, 0,
                              "illegal name in get_register_number(%s)", name);
        }
        return i;
}

static const char *
sample_core_get_register_name(conf_object_t *obj, int reg)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        register_table *rt = associated_register_table(core);

        if (reg < 0 || reg >= VLEN(*rt)) {
                SIM_LOG_ERROR(core->obj, 0,
                              "illegal register in get_register_name(%d)",
                              reg);
                return NULL;
        }
        register_description_t *rd = &VGET(*rt, reg);
        return rd->name;
}

static uint64
sample_core_read_register(conf_object_t *obj, int reg)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        register_table *rt = associated_register_table(core);

        if (reg < 0 || reg >= VLEN(*rt)) {
                SIM_LOG_ERROR(core->obj, 0,
                              "illegal register in read_register(%d)", reg);
                return 0;
        }
        register_description_t *rd = &VGET(*rt, reg);
        return rd->get_reg_value(core, rd->reg_number);
}

static void
sample_core_write_register(conf_object_t *obj, int reg, uint64 val)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        register_table *rt = associated_register_table(core);

        if (reg < 0 || reg >= VLEN(*rt)) {
                SIM_LOG_ERROR(core->obj, 0,
                              "illegal register in write_register(%d)", reg);
                return;
        }
        register_description_t *rd = &VGET(*rt, reg);
        rd->set_reg_value(core, rd->reg_number, val);
}


static attr_value_t
sample_core_all_registers(conf_object_t *obj)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        register_table *rt = associated_register_table(core);

        /* The sample_core_all_registers should return a list of
         * integers representing all the registers.  In our case,
         * that is then all the integers from 1 to the number of
         * registers in the register table. */

        attr_value_t ret = SIM_alloc_attr_list(VLEN(*rt));
        VFORI(*rt, i) {
                SIM_attr_list_set_item(&ret, i, SIM_make_attr_uint64(i));
        }
        return ret;
}

static int
sample_core_register_info(conf_object_t *obj, int reg,
                          ireg_info_t info)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        register_table *rt = associated_register_table(core);
        if (reg < 0 || reg >= VLEN(*rt)) {
                SIM_LOG_ERROR(core->obj, 0,
                              "illegal register in register_info(reg=%d)",
                              reg);
                return -1;
        }

        switch (info) {
        case Sim_RegInfo_Catchable:
                if (VGET(*rt, reg).catchable)
                        return 1;
                else
                        return 0;
        default:
                SIM_LOG_ERROR(core->obj, 0,
                              "illegal info type in register_info(info=%d)",
                              info);
                return -1;
        }
}

/*
 * exceptions
 */
#define Sample_Core_Number_Of_Exc 5
static const char *const sample_core_exception_names[] = {
        "Reset",
        "Data Abort",
        "Interrupt Request",
        "Undefined Instruction",
        "Software Interrupt"
};

int
sample_core_get_exception_number(conf_object_t *NOTNULL obj,
                                 const char *NOTNULL name)
{
        for (int i = 0; i < Sample_Core_Number_Of_Exc; i++) {
                if (strcmp(name, sample_core_exception_names[i]) == 0)
                        return i;
        }
        return -1;
}

const char *
sample_core_get_exception_name(conf_object_t *NOTNULL obj, int exc)
{
        if (exc < 0 || exc >= Sample_Core_Number_Of_Exc) {
                return NULL;
        }
        return sample_core_exception_names[exc];
}

int
sample_core_get_source(conf_object_t *NOTNULL obj, int exc)
{
        return -1;
}

attr_value_t
sample_core_all_exceptions(conf_object_t *NOTNULL obj)
{
        attr_value_t ret = SIM_alloc_attr_list(Sample_Core_Number_Of_Exc);
        for (unsigned i = 0; i < Sample_Core_Number_Of_Exc; i++)
                SIM_attr_list_set_item(&ret, i, SIM_make_attr_uint64(i));
        return ret;
}

static void
sample_core_ext_irq_raise(conf_object_t *obj)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        SIM_LOG_INFO(2, core->obj, 0,
                     "EXTERNAL_INTERRUPT raised.");
}

static void
sample_core_ext_irq_lower(conf_object_t *obj)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        SIM_LOG_INFO(2, core->obj, 0,
                     "EXTERNAL_INTERRUPT lowered.");
}

static void *
sample_core_init_object(conf_object_t *obj, void *data)
{
        sample_risc_t *core = MM_ZALLOC(1, sample_risc_t);
        core->obj = obj;

        core->enabled = 1;

        return core;
}

static void
sample_core_finalize(conf_object_t *obj)
{
        // do nothing
}


conf_class_t *
cr_define_class(void)
{
        return SIM_register_class(
                "sample-risc-core", &(class_data_t){
                  .init_object = sample_core_init_object,
                  .finalize_instance = sample_core_finalize,
                  .description = "Sample RISC core."
               });
}


void
cr_register_interfaces(conf_class_t *cr_class)
{
        static const context_handler_interface_t context_handler_iface = {
                .set_current_context = set_current_context,
                .get_current_context = get_current_context
        };
        SIM_register_context_handler(cr_class, &context_handler_iface);

        static const processor_info_v2_interface_t info_iface = {
                .disassemble = sample_core_disassemble,
                .set_program_counter = sample_core_set_program_counter,
                .get_program_counter = sample_core_get_program_counter,
                .logical_to_physical = sample_core_logical_to_physical_block,
                .get_processor_mode = sample_core_get_processor_mode,
                .enable_processor = sample_core_enable_processor,
                .disable_processor = sample_core_disable_processor,
                .get_enabled = sample_core_iface_get_enabled,
                .get_endian = sample_core_get_endian,
                .get_physical_memory = sample_core_get_physical_memory_iface,
                .get_logical_address_width =
                        sample_core_get_logical_address_width,
                .get_physical_address_width =
                        sample_core_get_physical_address_width,
                .architecture = sample_core_architecture
        };
        SIM_register_interface(
                cr_class, PROCESSOR_INFO_V2_INTERFACE, &info_iface);
        SIM_register_compatible_interfaces(
                cr_class, PROCESSOR_INFO_V2_INTERFACE);

        static const int_register_interface_t int_iface = {
                .get_number = sample_core_get_register_number,
                .get_name = sample_core_get_register_name,
                .read = sample_core_read_register,
                .write = sample_core_write_register,
                .all_registers = sample_core_all_registers,
                .register_info = sample_core_register_info
        };
        SIM_register_interface(cr_class, INT_REGISTER_INTERFACE, &int_iface);

        static const exception_interface_t exc_iface = {
                .get_number = sample_core_get_exception_number,
                .get_name = sample_core_get_exception_name,
                .get_source = sample_core_get_source,
                .all_exceptions = sample_core_all_exceptions
        };
        SIM_register_interface(cr_class, EXCEPTION_INTERFACE, &exc_iface);

        static const signal_interface_t ext_irq_iface = {
                .signal_raise = sample_core_ext_irq_raise,
                .signal_lower = sample_core_ext_irq_lower
        };
        SIM_register_port_interface(cr_class, SIGNAL_INTERFACE,
                                    &ext_irq_iface,
                                    "EXTERNAL_INTERRUPT",
                                    "External interrupt line.");
}

void
cr_register_attributes(conf_class_t *cr_class)
{

		SIM_register_typed_attribute(
				cr_class, "registers",
				toy_get_registers, NULL,
				toy_set_registers, NULL,
				Sim_Attr_Optional,
				"[i*]", NULL,
				"The registers.");

		SIM_register_typed_attribute(
				cr_class, "freq_mhz",
				toy_get_freq_mhz, NULL,
				toy_set_freq_mhz, NULL,
				Sim_Attr_Optional,
				"f", NULL,
				"The frequency in MHz for the core.");

		hap_Control_Register_Read =
				SIM_hap_get_number("Core_Control_Register_Read");
		hap_Control_Register_Write =
				SIM_hap_get_number("Core_Control_Register_Write");
		hap_Magic_Instruction = SIM_hap_get_number("Core_Magic_Instruction");

		/* declare each register to the world */ // FIXME where should this code be?
//        for (int k = 0; k < 16; k++) {
//                char name[8];
//                sprintf(name, "r%d", k);
//                sample_core_add_register_declaration(crc, name, k,
//                                                     toy_get_gpr,
//                                                     toy_set_gpr, 0);
//        }
//        sample_core_add_register_declaration(crc, "pc", 0, toy_read_pc,
//                                             toy_write_pc, 0);
//        sample_core_add_register_declaration(crc, "msr", 0, toy_read_msr,
//                                             toy_write_msr, 1);


        SIM_register_typed_attribute(
                cr_class, "physical_memory_space",
                sample_core_get_physical_memory, NULL,
                sample_core_set_physical_memory, NULL,
                Sim_Attr_Required,
                "o", NULL,
                "Physical memory space.");

        SIM_register_typed_attribute(
                cr_class, "core_enabled",
                sample_core_get_enabled, NULL,
                sample_core_set_enabled, NULL,
                Sim_Attr_Optional,
                "b", NULL,
                "Core state: enabled.");

        SIM_register_typed_attribute(
                cr_class, "idle_cycles",
                get_idle_cycles, NULL,
                set_idle_cycles, NULL,
                Sim_Attr_Optional,
                "i", NULL,
                "Number of idle cycles.");
}

static attr_value_t
sample_core_get_register_value(void *arg, conf_object_t *obj,
                               attr_value_t *idx)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        register_table *rt = associated_register_table(core);
        register_description_t *rd;

        if (arg != NULL) {
                rd = (register_description_t *)(arg);
        } else if (SIM_attr_is_integer(*idx)) {
                int n = SIM_attr_integer(*idx);
                if ((n < 0) || (n >= VLEN(*rt)))
                        return SIM_make_attr_invalid();
                rd = &VGET(*rt, n);
        } else
                return sample_core_all_registers(obj);

        /* return the value of register i */
        uint64 value = rd->get_reg_value(core, rd->reg_number);
        return SIM_make_attr_uint64(value);
}

static set_error_t
sample_core_set_register_value(void *arg, conf_object_t *obj,
                               attr_value_t *val, attr_value_t *idx)
{
        sample_risc_t *core = conf_obj_to_sr_core(obj);
        register_table *rt = associated_register_table(core);
        register_description_t *rd;

        if (arg != NULL) {
                rd = (register_description_t *)(arg);
        } else if (SIM_attr_is_nil(*idx)) {
                /* force all the registers to one value? */
                uint64 value = SIM_attr_integer(*val);
                VFORI(*rt, i) {
                        rd = &VGET(*rt, i);
                        rd->set_reg_value(core, rd->reg_number, value);
                }
                return Sim_Set_Ok;
        } else {
                int n = SIM_attr_integer(*idx);
                if ((n < 0) || (n >= VLEN(*rt))) {
                        SIM_attribute_error("Index out of range");
                        return Sim_Set_Object_Not_Found;
                }
                rd = &VGET(*rt, n);
        }

        /* set the value of register i to the "val" parameter */
        uint64 value = SIM_attr_integer(*val);
        rd->set_reg_value(core, rd->reg_number, value);
        return Sim_Set_Ok;
}

/* functions that interface between a call by the core
   and the actual function in the processor */
int
sample_core_logical_to_physical(sample_risc_t *core,
                                logical_address_t la_addr,
                                access_t access_type,
                                physical_address_t *pa_addr)
{
        int rc = toy_logical_to_physical(core, la_addr, pa_addr, access_type);
        if (!rc) {
                SIM_LOG_ERROR(core->obj, 0,
                              "logical address 0x%llx to physical address"
                              " failed", la_addr);
        } else {
                SIM_LOG_INFO(2, core->obj, 0,
                             "logical address 0x%llx to physical address"
                             " 0x%llx", la_addr, *pa_addr);
        }
        return rc;
}


/* get the current program counter for the core */
logical_address_t
sample_core_get_pc(sample_risc_t *core)
{
        return toy_get_pc(core);
}

/* set the current program counter for the core */
void
sample_core_set_pc(sample_risc_t *core, logical_address_t value)
{
        toy_set_pc(core, value);
}

void
sample_core_fetch_and_execute_instruction(sample_risc_t *core)
{
        toy_fetch_and_execute_instruction(core);
}

/* functions that interface between a call by the processor
   and the actual function in the core */
cycles_t
queue_next_event(event_queue_t *queue)
{
        if (queue_is_empty(queue))
                return 1000;
        else
                return get_delta(queue);
}

void
sample_core_increment_cycles(sample_risc_t *core, int cycles)
{
        sample_risc_t *sr = associated_queue(core);
        if (cycles > 0) {
                queue_decrement(&(sr->cycle_queue), cycles);
                sr->current_cycle += cycles;
        }
}

void
sample_core_increment_steps(sample_risc_t *core, int steps)
{
        sample_risc_t *sr = associated_queue(core);
        if (steps > 0) {
                queue_decrement(&(sr->step_queue), steps);
                sr->current_step += steps;
        }
}

int
sample_core_next_cycle_event(sample_risc_t *core)
{
        sample_risc_t *sr = associated_queue(core);
        int cycles = queue_next_event(&sr->cycle_queue);
        return cycles;
}

int
sample_core_next_step_event(sample_risc_t *core)
{
        sample_risc_t *sr = associated_queue(core);
        int steps;
        steps = queue_next_event(&sr->step_queue);
        return steps;
}


execute_state_t
sample_core_state(sample_risc_t *core)
{
        sample_risc_t *sr = associated_queue(core);
        return sr->state;
}

void
sample_core_check_virtual_breakpoints(sample_risc_t *core,
                                      access_t access,
                                      logical_address_t virt_start,
                                      generic_address_t len,
                                      uint8 *data)
{
        sample_risc_t *sr = associated_memory(core);
        check_virtual_breakpoints(sr, core, access, virt_start, len, data);
}


bool
sample_core_fetch_instruction(sample_risc_t *core,
                              physical_address_t pa, physical_address_t len,
                              uint8 *data, int check_bp)
{
        sample_risc_t *sr = associated_memory(core);
        return fetch_instruction(sr, core, pa, len, data, check_bp);
}


bool
sample_core_write_memory(sample_risc_t *core,
                         physical_address_t pa, physical_address_t len,
                         uint8 *data, int check_bp)
{
        sample_risc_t *sr = associated_memory(core);
        return write_memory(sr, core, pa, len, data, check_bp);
}

bool
sample_core_read_memory(sample_risc_t *core,
                        physical_address_t pa, physical_address_t len,
                        uint8 *data, int check_bp)
{
        sample_risc_t *sr = associated_memory(core);
        return read_memory(sr, core, pa, len, data, check_bp);
}

void
sample_core_release_and_share(sample_risc_t *core,
                              physical_address_t phys_address)
{
        sample_risc_t *sr = associated_memory(core);
        release_and_share(sr, core, phys_address);
}

static void
sr_cell_change(lang_void *callback_data, conf_object_t *obj,
               conf_object_t *old_cell, conf_object_t *new_cell)
{
        sample_risc_t *sr = conf_obj_to_sr(obj);
        if (sr->cell) {
                const simple_dispatcher_interface_t *iface =
                        SIM_c_get_port_interface(sr->cell,
                                                 SIMPLE_DISPATCHER_INTERFACE,
                                                 "breakpoint_change");
                ASSERT(iface);
                iface->unsubscribe(sr->cell, obj, NULL);
        }

        sr->cell = new_cell;
        if (sr->cell) {
                sr->cell_iface = SIM_get_interface(new_cell,
                                                   CELL_INSPECTION_INTERFACE);

                const simple_dispatcher_interface_t *iface =
                        SIM_c_get_port_interface(sr->cell,
                                                 SIMPLE_DISPATCHER_INTERFACE,
                                                 "breakpoint_change");
                ASSERT(iface);
                iface->subscribe(sr->cell, obj, NULL);
        } else {
                sr->cell_iface = NULL;
        }
}

static void *
sr_init_object(conf_object_t *obj, void *data)
{
        sample_risc_t *sr = MM_ZALLOC(1, sample_risc_t);
        sr->obj = obj;
        VINIT(sr->cached_pages);
        instantiate_step_queue(sr);
        instantiate_cycle_queue(sr);
        instantiate_frequency(sr);

        SIM_hap_add_callback_obj("Core_Conf_Clock_Change_Cell", sr->obj, 0,
                                 (obj_hap_func_t)sr_cell_change, NULL);

        return sr;
}

static void
sr_finalize(conf_object_t *obj)
{
        sample_risc_t *sr = conf_obj_to_sr(obj);
        finalize_frequency(sr);
}

conf_class_t *
sr_define_class(void)
{
        return SIM_register_class(
                "sample-risc",
                &(class_data_t){
                  .init_object = sr_init_object,
                  .finalize_instance = sr_finalize,
                  .description =
                         "The sample RISC"
                         " (Reduced Instructions Set Computer/Cosimulator)."
                });
}

void
sr_register_attributes(conf_class_t *sr_class)
{

}

void
sample_risc_cycle_event_posted(sample_risc_t *sr)
{
        /* An event has been posted. We only single step, so we don't need to
           bother. */
}

void
sample_risc_step_event_posted(sample_risc_t *sr)
{
        /* An event has been posted. We only single step, so we don't need to
           bother. */
}

/*
 * Called once when the device module is loaded into Simics.
 */
void
init_local(void)
{
        conf_class_t *sr_class = sr_define_class();
        sr_register_attributes(sr_class);
        register_memory_interfaces(sr_class);
        register_memory_attributes(sr_class);
        register_execute_interface(sr_class);
        register_cycle_queue(sr_class);
        register_step_queue(sr_class);
        register_frequency_interfaces(sr_class);

        /* initialize the register table */
        /* must be done before we allow the processor to initialize */
//        init_register_table(crc);

        cr_register_attributes(sr_class);
        cr_register_interfaces(sr_class);

        // FIXME add static register registration
//        VFORI(*rt, i) {
//                register_description_t *rd;
//                rd = &VGET(*rt, i);
//
//                SIM_register_typed_attribute(crc->cr_class, rd->name,
//                                             sample_core_get_register_value,
//                                             rd,
//                                             sample_core_set_register_value,
//                                             rd,
//                                             Sim_Attr_Optional,
//                                             "i", NULL,
//                                             rd->name);
//        }

}

