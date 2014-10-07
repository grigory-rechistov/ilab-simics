/*
  chip16.c - CHIP16 CPU model

  This Software is part of Wind River Simics. The rights to copy, distribute,
  modify, or otherwise make use of this Software may be licensed only
  pursuant to the terms of an applicable Wind River license agreement.
  
  Copyright 2010-2014 Intel Corporation

*/

#include "chip16.h"
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
#include "chip16.h"
#include "chip16-memory.h"
#include "chip16-queue.h"

// TODO: should not be needed, violates the Processor API
#include <simics/simulator/hap-consumer.h>

#include "event-queue.h"
#include "chip16-memory.h"
#include "chip16-exec.h"
#include "chip16-cycle.h"
#include "chip16-step.h"
#include "chip16-frequency.h"

/* fixed size instructions of 4 bytes */
#define INSTR_SIZE 4

#define MAX_REG 0xffff

/*
 * TODO: Expand me
 *
 *           31   28 27   24 23   20 19   16 15   12 11    8 7     4 3     0
 *          |-------|-------|-------|-------|-------|-------|-------|-------|
 *               opcode     |   Y   |   X   |      LL       |      HH       |
 */

#define INSTR_OP(i)       (((i) >> 24) & 0xff)
#define INSTR_SRC_REG(i)  (((i) >> 20) & 0xf)
#define INSTR_DST_REG(i)  (((i) >> 16) & 0xf)
#define INSTR_LL          (((i) >> 8) & 0xff)
#define INSTR_HH          ((i) & 0xff)

// TODO: Expand me
typedef enum {
        Instr_Op_Nop    = 0,
} instr_op_t;

/* THREAD_SAFE_GLOBAL: hap_Control_Register_Read init */
hap_type_t hap_Control_Register_Read;
/* THREAD_SAFE_GLOBAL: hap_Control_Register_Write init */
hap_type_t hap_Control_Register_Write;
/* THREAD_SAFE_GLOBAL: hap_Magic_Instruction init */
hap_type_t hap_Magic_Instruction;

/*
 * registers
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
        Reg_Idx_RA,
        Reg_Idx_RB,
        Reg_Idx_RC,
        Reg_Idx_RD,
        Reg_Idx_RE,
        Reg_Idx_RF,
        Reg_Idx_PC,
        Num_Regs        /* keep it last */
} chip16_reg_idx_t;

const char *const reg_names[Num_Regs] = {
        [Reg_Idx_R0] = "r0",
        [Reg_Idx_R1] = "r1",
        [Reg_Idx_R2] = "r2",
        [Reg_Idx_R3] = "r3",
        [Reg_Idx_R4] = "r4",
        [Reg_Idx_R5] = "r5",
        [Reg_Idx_R6] = "r6",
        [Reg_Idx_R7] = "r7",
        [Reg_Idx_R8] = "r8",
        [Reg_Idx_R9] = "r9",
        [Reg_Idx_RA] = "ra",
        [Reg_Idx_RB] = "rb",
        [Reg_Idx_RC] = "rc",
        [Reg_Idx_RD] = "rd",
        [Reg_Idx_RE] = "re",
        [Reg_Idx_RF] = "rf",
        [Reg_Idx_PC] = "pc"
};

static attr_value_t
chip16_get_reg(void *reg, conf_object_t *obj, attr_value_t *idx)
{
        chip16_t *cpu = conf_to_chip16(obj);
        chip16_reg_idx_t id = (chip16_reg_idx_t)reg;
        uint16 value;
        switch (id) {
        case Reg_Idx_PC:
                value = cpu->chip16_pc;
                break;
        default:
                ASSERT(0);
        }
        return SIM_make_attr_uint64(value);
}

static set_error_t
chip16_set_reg(void *reg, conf_object_t *obj,
               attr_value_t *val, attr_value_t *idx)
{
        chip16_t *cpu = conf_to_chip16(obj);
        chip16_reg_idx_t id = (chip16_reg_idx_t)reg;
        uint16 value = SIM_attr_integer(*val);
        set_error_t ret = Sim_Set_Ok;
        switch (id) {
        case Reg_Idx_PC:
                if (value % INSTR_SIZE)
                        ret = Sim_Set_Illegal_Value;
                else
                        cpu->chip16_pc = value;
                break;
        default:
                ASSERT(0);
        }
        return ret;
}

logical_address_t
chip16_get_pc(chip16_t *core)
{
        return core->chip16_pc;
}

void
chip16_set_pc(chip16_t *core, logical_address_t pc)
{
        core->chip16_pc = pc;
}

/* get gpr[i] for the core */
uint64
chip16_get_gpr(chip16_t *core, int i)
{
        return core->chip16_reg[i];
}

/* set gpr[i] for the core */
void
chip16_set_gpr(chip16_t *core, int i, uint64 value)
{
        core->chip16_reg[i] = value;
}

void
chip16_init_registers(void *crc)
{
        /*
         * keep in sync with chip16_set/get_registers
         */
}

uint32
chip16_read_memory32(chip16_t *core, logical_address_t la,
                          physical_address_t pa)
{
        uint32 data = 0;
        chip16_check_virtual_breakpoints(
                core, Sim_Access_Read, la, 4, (uint8 *)(&data));
        chip16_read_memory(core, pa, 4, (uint8 *)&data, 1);
        return UNALIGNED_LOAD_LE32(&data);
}

void
chip16_write_memory32(chip16_t *core, logical_address_t la,
                           physical_address_t pa, uint32 value)
{
        uint32 data;
        UNALIGNED_STORE_LE32(&data, value);
        chip16_check_virtual_breakpoints(
                core, Sim_Access_Write, la, 4, (uint8 *)(&data));
        chip16_write_memory(core, pa, 4, (uint8 *)&data, 1);

        /* Hint to the simulator that a page is likely to be
           sharable. The heuristic here is that a write to the last
           location on the page may be from a memset, and that's a
           good time to share the page. This works well for simple
           linear access patterns. */
        if ((pa & (4096-1)) == 4092)
                chip16_release_and_share(core, pa);
}

char *
chip16_string_decode(chip16_t *core, uint32 instr)
{
        switch (INSTR_OP(instr)) {

        case Instr_Op_Nop:
                return MM_STRDUP("nop");

        default:
                SIM_LOG_ERROR(core->obj, 0, "unknown instruction");
                return MM_STRDUP("unknown");
        }
}

#define INCREMENT_PC(core) chip16_set_pc(core, chip16_get_pc(core) + INSTR_SIZE)

void
chip16_execute(chip16_t *core, uint32 instr)
{
        switch (INSTR_OP(instr)) {

        case Instr_Op_Nop:
                chip16_increment_cycles(core, 1);
                chip16_increment_steps(core, 1);
                INCREMENT_PC(core);
                break;

        default:
                SIM_LOG_ERROR(core->obj, 0,
                              "unknown instruction");
                break;
        }
}

void
chip16_fetch_and_execute_instruction(chip16_t *core)
{
        /* get instruction */
        logical_address_t pc = chip16_get_pc(core);
        physical_address_t pa;

        int ok = chip16_logical_to_physical(core, pc, Sim_Access_Execute, &pa);
        if (ok) {
                uint32 instr;
                chip16_fetch_instruction(core, pa, INSTR_SIZE,
                                              (uint8 *)(&instr), true);
                chip16_check_virtual_breakpoints(
                        core, Sim_Access_Execute, pc, 4, (uint8 *)(&instr));
                instr = UNALIGNED_LOAD_LE32(&instr);
                if (chip16_state(core) != State_Stopped) {
                        /* breakpoint triggered - execute instruction */
                        chip16_execute(core, instr);
                }
        }
}

/*
 * physical_memory_space attribute functions
 */
static attr_value_t
chip16_get_physical_memory(void *arg, conf_object_t *obj,
                                attr_value_t *idx)
{
        chip16_t *core = conf_to_chip16(obj);
        return SIM_make_attr_object(core->phys_mem_obj);
}

static set_error_t
chip16_set_physical_memory(void *arg, conf_object_t *obj,
                                attr_value_t *val, attr_value_t *idx)
{
        chip16_t *core = conf_to_chip16(obj);
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
chip16_get_enabled(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        chip16_t *core = conf_to_chip16(obj);
        return SIM_make_attr_boolean(core->enabled);
}

static set_error_t
chip16_set_enabled(void *arg, conf_object_t *obj,
                        attr_value_t *val, attr_value_t *idx)
{
        chip16_t *core = conf_to_chip16(obj);
        core->enabled = SIM_attr_boolean(*val);
        return Sim_Set_Ok;
}

/*
 * idle_cycles attribute functions
 */
static attr_value_t
get_idle_cycles(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        chip16_t *core = conf_to_chip16(obj);
        return SIM_make_attr_uint64(core->idle_cycles);
}

static set_error_t
set_idle_cycles(void *arg, conf_object_t *obj,
                attr_value_t *val, attr_value_t *idx)
{
        chip16_t *core = conf_to_chip16(obj);
        core->idle_cycles = SIM_attr_integer(*val);
        return Sim_Set_Ok;
}

/*
 * context_handler interface functions
 */
static conf_object_t *
get_current_context(conf_object_t *obj)
{
        chip16_t *core = conf_to_chip16(obj);
        return core->current_context;
}

static int
set_current_context(conf_object_t *obj, conf_object_t *ctx)
{
        chip16_t *core = conf_to_chip16(obj);
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
chip16_disassemble(conf_object_t *obj, generic_address_t address,
                        attr_value_t instruction_data, int sub_operation)
{
        chip16_t *core = conf_to_chip16(obj);

        SIM_LOG_INFO(2, core->obj, 0,
                     "disassembling instruction at address 0x%llx", address);
        if (sub_operation != 0 && sub_operation != -1)
                return (tuple_int_string_t){0, NULL};

        if (SIM_attr_data_size(instruction_data) < 4) {
                /* The opcode is not complete. Tell the caller that we need 4
                   bytes. */
                return (tuple_int_string_t){-4, NULL};
        }

        uint32 instr = UNALIGNED_LOAD_LE32(SIM_attr_data(instruction_data));
        return (tuple_int_string_t){4, chip16_string_decode(core, instr)};
}

static void
chip16_set_program_counter(conf_object_t *obj,
                                logical_address_t pc)
{
        chip16_t *core = conf_to_chip16(obj);
        SIM_LOG_INFO(2, core->obj, 0,
                     "setting program counter to 0x%llx", pc);
        chip16_set_pc(core, pc);
}

static logical_address_t
chip16_get_program_counter(conf_object_t *obj)
{
        chip16_t *core = conf_to_chip16(obj);
        return chip16_get_pc(core);
}

static physical_block_t
chip16_logical_to_physical_block(conf_object_t *obj,
                                      logical_address_t address,
                                      access_t access_type)
{
        chip16_t *core = conf_to_chip16(obj);
        physical_block_t ret;
        ret.valid = chip16_logical_to_physical(core, address, access_type,
                                                    &ret.address);
        ret.block_start = ret.address;
        ret.block_end = ret.address;
        SIM_LOG_INFO(2, core->obj, 0,
                     "logical address 0x%llx to physical address 0x%llx",
                     address, ret.address);
        return ret;
}

static processor_mode_t
chip16_get_processor_mode(conf_object_t *obj)
{
        return Sim_CPU_Mode_User;  /* we don't have different modes */
}

static int
chip16_enable_processor(conf_object_t *obj)
{
        chip16_t *core = conf_to_chip16(obj);
        if (core->enabled)
                return 1;
        core->enabled = 1;
        return 0;
}

static int
chip16_disable_processor(conf_object_t *obj)
{
        chip16_t *core = conf_to_chip16(obj);
        if (!core->enabled)
                return 1;
        core->enabled = 0;
        return 0;
}

static int
chip16_iface_get_enabled(conf_object_t *obj)
{
        chip16_t *core = conf_to_chip16(obj);
        return core->enabled;
}

static cpu_endian_t
chip16_get_endian(conf_object_t *obj)
{
        return Sim_Endian_Little;
}

static conf_object_t *
chip16_get_physical_memory_iface(conf_object_t *obj)
{
        chip16_t *core = conf_to_chip16(obj);
        return core->phys_mem_obj;
}

static int
chip16_get_logical_address_width(conf_object_t *obj)
{
        return 16;
}

static int
chip16_get_physical_address_width(conf_object_t *obj)
{
        return 16;
}

static const char *
chip16_architecture(conf_object_t *obj)
{
        return "chip16";
}

/*
 * int_register_interface
 */

static int
chip16_get_register_number(conf_object_t *obj, const char *name)
{
        size_t i;
        for (i = 0; i < Num_Regs; i++) {
                if (reg_names[i] && strcasecmp(name, reg_names[i]) == 0)
                        return i;
        }

        return -1; /* Not found */
}

static const char *
chip16_get_register_name(conf_object_t *obj, int reg)
{
        if (reg < 0 || reg > Num_Regs)
                return NULL;

        return reg_names[reg];
}

static uint64
chip16_read_register(conf_object_t *obj, int reg)
{
        chip16_t *core = conf_to_chip16(obj);
        if (reg < 0 || reg > Num_Regs) {
                SIM_LOG_INFO(1, core->obj, 0,
                             "Illegal register in read_register(reg=%d).", reg);
                return 0;
        }

        switch (reg) {
        case Reg_Idx_PC:
                return chip16_get_pc(core);
        default:
                SIM_LOG_INFO(1, core->obj, 0,
                             "read_register(reg=%d) not handled, returning 0.",
                             reg);
                return 0;
        }
}

static void
chip16_write_register(conf_object_t *obj, int reg, uint64 val)
{
        chip16_t *core = conf_to_chip16(obj);
        if (reg < 0 || reg > Num_Regs) {
                SIM_LOG_INFO(1, core->obj, 0,
                             "Illegal register in write_register(reg=%d).",
                             reg);
                return;
        }

        if (val > MAX_REG) {
                SIM_LOG_INFO(1, core->obj, 0,
                             "Too large value %#llx. Ignored.", val);
                return;
        }

        switch (reg)  {
        case Reg_Idx_PC:
                chip16_set_pc(core, val);
                break;
        default:
                SIM_LOG_INFO(1, core->obj, 0,
                             "write_register(reg=%d) unknown register.", reg);
        }
}


static attr_value_t
chip16_all_registers(conf_object_t *obj)
{
        chip16_t *core = conf_to_chip16(obj);
        attr_value_t ret = SIM_alloc_attr_list(Num_Regs);

        for (size_t i = Reg_Idx_R0; i < Reg_Idx_RF; i++)
                SIM_attr_list_set_item(
                        &ret, i,
                        SIM_make_attr_uint64(core->chip16_reg[i - Reg_Idx_R0]));

        SIM_attr_list_set_item(&ret, Reg_Idx_PC,
                               SIM_make_attr_uint64(chip16_get_pc(core)));
        return ret;
}

static int
chip16_register_info(conf_object_t *obj, int reg,
                          ireg_info_t info)
{
        chip16_t *core = conf_to_chip16(obj);
        if (reg < 0 || reg >= Num_Regs) {
                SIM_LOG_ERROR(core->obj, 0,
                              "Illegal register in register_info(reg=%d)",
                              reg);
                return -1;
        }

        switch (info) {
        case Sim_RegInfo_Catchable:
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
#define chip16_Number_Of_Exc 5
static const char *const chip16_exception_names[] = {
        "Reset",
        "Data Abort",
        "Interrupt Request",
        "Undefined Instruction",
        "Software Interrupt"
};

int
chip16_get_exception_number(conf_object_t *NOTNULL obj,
                                 const char *NOTNULL name)
{
        for (int i = 0; i < chip16_Number_Of_Exc; i++) {
                if (strcmp(name, chip16_exception_names[i]) == 0)
                        return i;
        }
        return -1;
}

const char *
chip16_get_exception_name(conf_object_t *NOTNULL obj, int exc)
{
        if (exc < 0 || exc >= chip16_Number_Of_Exc) {
                return NULL;
        }
        return chip16_exception_names[exc];
}

int
chip16_get_source(conf_object_t *NOTNULL obj, int exc)
{
        return -1;
}

attr_value_t
chip16_all_exceptions(conf_object_t *NOTNULL obj)
{
        attr_value_t ret = SIM_alloc_attr_list(chip16_Number_Of_Exc);
        for (unsigned i = 0; i < chip16_Number_Of_Exc; i++)
                SIM_attr_list_set_item(&ret, i, SIM_make_attr_uint64(i));
        return ret;
}

static void
chip16_vblank_raise(conf_object_t *obj)
{
        chip16_t *core = conf_to_chip16(obj);
        SIM_LOG_INFO(1, core->obj, 0, "VBLANK raised. TODO enable core");
}

static void
chip16_vblank_lower(conf_object_t *obj)
{
        chip16_t *core = conf_to_chip16(obj);
        SIM_LOG_INFO(1, core->obj, 0, "VBLANK lowered.");
}

static void
chip16_cell_change(lang_void *callback_data, conf_object_t *obj,
               conf_object_t *old_cell, conf_object_t *new_cell)
{
        chip16_t *sr = conf_to_chip16(obj);
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
chip16_init_object(conf_object_t *obj, void *data)
{
        chip16_t *core = MM_ZALLOC(1, chip16_t);
        core->obj = obj;

        VINIT(core->cached_pages);
        instantiate_step_queue(core);
        instantiate_cycle_queue(core);
        instantiate_frequency(core);

        SIM_hap_add_callback_obj("Core_Conf_Clock_Change_Cell", core->obj, 0,
                                 (obj_hap_func_t)chip16_cell_change, NULL);

        core->enabled = 1;

        return core;
}

static void
chip16_finalize(conf_object_t *obj)
{
        chip16_t *sr = conf_to_chip16(obj);
        finalize_frequency(sr);
}

conf_class_t *
chip16_define_class(void)
{
        return SIM_register_class(
                "chip16", &(class_data_t){
                  .init_object = chip16_init_object,
                  .finalize_instance = chip16_finalize,
                  .description = "CHIP16 core."
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
                .disassemble = chip16_disassemble,
                .set_program_counter = chip16_set_program_counter,
                .get_program_counter = chip16_get_program_counter,
                .logical_to_physical = chip16_logical_to_physical_block,
                .get_processor_mode = chip16_get_processor_mode,
                .enable_processor = chip16_enable_processor,
                .disable_processor = chip16_disable_processor,
                .get_enabled = chip16_iface_get_enabled,
                .get_endian = chip16_get_endian,
                .get_physical_memory = chip16_get_physical_memory_iface,
                .get_logical_address_width =
                        chip16_get_logical_address_width,
                .get_physical_address_width =
                        chip16_get_physical_address_width,
                .architecture = chip16_architecture
        };
        SIM_register_interface(
                cr_class, PROCESSOR_INFO_V2_INTERFACE, &info_iface);
        SIM_register_compatible_interfaces(
                cr_class, PROCESSOR_INFO_V2_INTERFACE);

        static const int_register_interface_t int_iface = {
                .get_number = chip16_get_register_number,
                .get_name = chip16_get_register_name,
                .read = chip16_read_register,
                .write = chip16_write_register,
                .all_registers = chip16_all_registers,
                .register_info = chip16_register_info
        };
        SIM_register_interface(cr_class, INT_REGISTER_INTERFACE, &int_iface);

        static const exception_interface_t exc_iface = {
                .get_number = chip16_get_exception_number,
                .get_name = chip16_get_exception_name,
                .get_source = chip16_get_source,
                .all_exceptions = chip16_all_exceptions
        };
        SIM_register_interface(cr_class, EXCEPTION_INTERFACE, &exc_iface);

        static const signal_interface_t vblank_iface = {
                .signal_raise = chip16_vblank_raise,
                .signal_lower = chip16_vblank_lower
        };
        SIM_register_port_interface(cr_class, SIGNAL_INTERFACE,
                                    &vblank_iface,
                                    "VBLANK",
                                    "Interrupt line for VBLANK.");
}

void
cr_register_attributes(conf_class_t *cr_class)
{
		hap_Control_Register_Read =
				SIM_hap_get_number("Core_Control_Register_Read");
		hap_Control_Register_Write =
				SIM_hap_get_number("Core_Control_Register_Write");
		hap_Magic_Instruction = SIM_hap_get_number("Core_Magic_Instruction");

        SIM_register_typed_attribute(
                cr_class, "pc",
                chip16_get_reg, (void *)Reg_Idx_PC,
                chip16_set_reg, (void *)Reg_Idx_PC,
                Sim_Attr_Optional,
                "i", NULL,
                "Programm counter.");

        SIM_register_typed_attribute(
                cr_class, "physical_memory_space",
                chip16_get_physical_memory, NULL,
                chip16_set_physical_memory, NULL,
                Sim_Attr_Required,
                "o", NULL,
                "Physical memory space.");

        SIM_register_typed_attribute(
                cr_class, "core_enabled",
                chip16_get_enabled, NULL,
                chip16_set_enabled, NULL,
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

/* access_type is Sim_Access_Read, Sim_Access_Write, Sim_Access_Execute */
/* returns boolean */
int
chip16_logical_to_physical(chip16_t *core,
                           logical_address_t la_addr,
                           access_t access_type,
                           physical_address_t *pa_addr)
{
        /* no address mapping for the chip16 system */
        *pa_addr = la_addr;
        SIM_LOG_INFO(4, core->obj, 0,
                     "logical address 0x%llx to physical address"
                     " 0x%llx", la_addr, *pa_addr);
        return 1;
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
chip16_increment_cycles(chip16_t *core, int cycles)
{
        if (cycles > 0) {
                queue_decrement(&(core->cycle_queue), cycles);
                core->current_cycle += cycles;
        }
}

void
chip16_increment_steps(chip16_t *core, int steps)
{
        if (steps > 0) {
                queue_decrement(&(core->step_queue), steps);
                core->current_step += steps;
        }
}

int
chip16_next_cycle_event(chip16_t *core)
{
        int cycles = queue_next_event(&core->cycle_queue);
        return cycles;
}

int
chip16_next_step_event(chip16_t *core)
{
        int steps = queue_next_event(&core->step_queue);
        return steps;
}

execute_state_t
chip16_state(chip16_t *core)
{
        return core->state;
}

void
chip16_cycle_event_posted(chip16_t *sr)
{
        /* An event has been posted. We only single step, so we don't need to
           bother. */
}

void
chip16_step_event_posted(chip16_t *sr)
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
        conf_class_t *sr_class = chip16_define_class();
        register_memory_interfaces(sr_class);
        register_memory_attributes(sr_class);
        register_execute_interface(sr_class);
        register_cycle_queue(sr_class);
        register_step_queue(sr_class);
        register_frequency_interfaces(sr_class);

        cr_register_attributes(sr_class);
        cr_register_interfaces(sr_class);
}

