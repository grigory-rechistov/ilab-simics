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
 * keep in sync with chip16_init_registers
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
} register_index_t;

static attr_value_t
chip16_get_registers(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        chip16_t *core = conf_to_chip16(obj);
        attr_value_t ret = SIM_alloc_attr_list(16 + 2);

        /*
         * keep in sync with chip16_init_registers
         */
        for (unsigned i = 0; i < 16; i++)
                SIM_attr_list_set_item(
                        &ret, i, SIM_make_attr_uint64(core->chip16_reg[i]));

        SIM_attr_list_set_item(&ret, 16, SIM_make_attr_uint64(core->chip16_pc));
        return ret;
}

static set_error_t
chip16_set_registers(void *arg, conf_object_t *obj,
                  attr_value_t *val, attr_value_t *idx)
{
        chip16_t *core = conf_to_chip16(obj);

        /*
         * keep in sync with chip16_init_registers
         */
        if (SIM_attr_list_size(*val) < 18)
                return Sim_Set_Illegal_Value;
        for (unsigned i = 0; i < 16; i++)
                core->chip16_reg[i] =
                        SIM_attr_integer(SIM_attr_list_item(*val, i));
        core->chip16_pc = SIM_attr_integer(SIM_attr_list_item(*val, 16));
        return Sim_Set_Ok;
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
        return UNALIGNED_LOAD_BE32(&data);
}

void
chip16_write_memory32(chip16_t *core, logical_address_t la,
                           physical_address_t pa, uint32 value)
{
        uint32 data;
        UNALIGNED_STORE_BE32(&data, value);
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
                instr = UNALIGNED_LOAD_BE32(&instr);
                if (chip16_state(core) != State_Stopped) {
                        /* breakpoint triggered - execute instruction */
                        chip16_execute(core, instr);
                }
        }
}

static register_table *
associated_register_table(chip16_t *core)
{
        register_table *rt = &(core->reg_table);
        return rt;
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

        uint32 instr = UNALIGNED_LOAD_BE32(SIM_attr_data(instruction_data));
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

// TODO: is it needed?
//static void
//init_register_table(chip16_t *crc)
//{
//        register_table *rt = &(crc->reg_table);
//        VINIT(*rt);
//}

int
search_register_table(chip16_t *crc, const char *name)
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
chip16_add_register_declaration(chip16_t *core,
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
chip16_get_register_number(conf_object_t *obj, const char *name)
{
        chip16_t *core = conf_to_chip16(obj);
        int i = search_register_table(core, name);

        if (i == -1) {
                SIM_LOG_ERROR(core->obj, 0,
                              "illegal name in get_register_number(%s)", name);
        }
        return i;
}

static const char *
chip16_get_register_name(conf_object_t *obj, int reg)
{
        chip16_t *core = conf_to_chip16(obj);
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
chip16_read_register(conf_object_t *obj, int reg)
{
        chip16_t *core = conf_to_chip16(obj);
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
chip16_write_register(conf_object_t *obj, int reg, uint64 val)
{
        chip16_t *core = conf_to_chip16(obj);
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
chip16_all_registers(conf_object_t *obj)
{
        chip16_t *core = conf_to_chip16(obj);
        register_table *rt = associated_register_table(core);

        /* The chip16_all_registers should return a list of
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
chip16_register_info(conf_object_t *obj, int reg,
                          ireg_info_t info)
{
        chip16_t *core = conf_to_chip16(obj);
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
chip16_ext_irq_raise(conf_object_t *obj)
{
        chip16_t *core = conf_to_chip16(obj);
        SIM_LOG_INFO(2, core->obj, 0,
                     "EXTERNAL_INTERRUPT raised.");
}

static void
chip16_ext_irq_lower(conf_object_t *obj)
{
        chip16_t *core = conf_to_chip16(obj);
        SIM_LOG_INFO(2, core->obj, 0,
                     "EXTERNAL_INTERRUPT lowered.");
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

        static const signal_interface_t ext_irq_iface = {
                .signal_raise = chip16_ext_irq_raise,
                .signal_lower = chip16_ext_irq_lower
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
				chip16_get_registers, NULL,
				chip16_set_registers, NULL,
				Sim_Attr_Optional,
				"[i*]", NULL,
				"The registers.");

		hap_Control_Register_Read =
				SIM_hap_get_number("Core_Control_Register_Read");
		hap_Control_Register_Write =
				SIM_hap_get_number("Core_Control_Register_Write");
		hap_Magic_Instruction = SIM_hap_get_number("Core_Magic_Instruction");

		/* declare each register to the world */ // FIXME where should this code be?
//        for (int k = 0; k < 16; k++) {
//                char name[8];
//                sprintf(name, "r%d", k);
//                chip16_add_register_declaration(crc, name, k,
//                                                     chip16_get_gpr,
//                                                     chip16_set_gpr, 0);
//        }
//        chip16_add_register_declaration(crc, "pc", 0, chip16_read_pc,
//                                             chip16_write_pc, 0);


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

// TODO: is it needed?
//static attr_value_t
//chip16_get_register_value(void *arg, conf_object_t *obj,
//                               attr_value_t *idx)
//{
//        chip16_t *core = conf_to_chip16(obj);
//        register_table *rt = associated_register_table(core);
//        register_description_t *rd;
//
//        if (arg != NULL) {
//                rd = (register_description_t *)(arg);
//        } else if (SIM_attr_is_integer(*idx)) {
//                int n = SIM_attr_integer(*idx);
//                if ((n < 0) || (n >= VLEN(*rt)))
//                        return SIM_make_attr_invalid();
//                rd = &VGET(*rt, n);
//        } else
//                return chip16_all_registers(obj);
//
//        /* return the value of register i */
//        uint64 value = rd->get_reg_value(core, rd->reg_number);
//        return SIM_make_attr_uint64(value);
//}

// TODO: is it needed?
//static set_error_t
//chip16_set_register_value(void *arg, conf_object_t *obj,
//                               attr_value_t *val, attr_value_t *idx)
//{
//        chip16_t *core = conf_to_chip16(obj);
//        register_table *rt = associated_register_table(core);
//        register_description_t *rd;
//
//        if (arg != NULL) {
//                rd = (register_description_t *)(arg);
//        } else if (SIM_attr_is_nil(*idx)) {
//                /* force all the registers to one value? */
//                uint64 value = SIM_attr_integer(*val);
//                VFORI(*rt, i) {
//                        rd = &VGET(*rt, i);
//                        rd->set_reg_value(core, rd->reg_number, value);
//                }
//                return Sim_Set_Ok;
//        } else {
//                int n = SIM_attr_integer(*idx);
//                if ((n < 0) || (n >= VLEN(*rt))) {
//                        SIM_attribute_error("Index out of range");
//                        return Sim_Set_Object_Not_Found;
//                }
//                rd = &VGET(*rt, n);
//        }
//
//        /* set the value of register i to the "val" parameter */
//        uint64 value = SIM_attr_integer(*val);
//        rd->set_reg_value(core, rd->reg_number, value);
//        return Sim_Set_Ok;
//}

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
sr_register_attributes(conf_class_t *sr_class)
{

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
//                                             chip16_get_register_value,
//                                             rd,
//                                             chip16_set_register_value,
//                                             rd,
//                                             Sim_Attr_Optional,
//                                             "i", NULL,
//                                             rd->name);
//        }

}

