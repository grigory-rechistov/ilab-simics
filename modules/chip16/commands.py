# This Software is part of Wind River Simics. The rights to copy, distribute,
# modify, or otherwise make use of this Software may be licensed only
# pursuant to the terms of an applicable Wind River license agreement.
# 
# Copyright 2010-2014 Intel Corporation

from cli import *
from simics import *
import cli_impl
import sim_commands

def get_sample_cosimulator_info(obj):
    return []

def get_sample_cosimulator_status(obj):
    return []

new_info_command('sample-risc', get_sample_cosimulator_info)
new_status_command('sample-risc', get_sample_cosimulator_status)

def get_sample_core_info(obj):
    return []

def get_sample_core_status(obj):
    return []

new_info_command("sample-risc-core", get_sample_core_info)
new_status_command("sample-risc-core", get_sample_core_status)

# Function called by the 'pregs' command. Print common registers if
# all is false, and print more registers if all is true.
def local_pregs(obj, all):
    return "pc = 0x%x" % obj.iface.processor_info.get_program_counter()

# Function used to track register changes when using stepi -r.
def local_diff_regs(obj):
    return ()

# Function used by default disassembler to indicate that the next
# step in the system will be an exception/interrupt.
def local_pending_exception(obj):
    return None

processor_cli_iface = processor_cli_interface_t()
processor_cli_iface.get_disassembly = sim_commands.make_disassembly_fun()
processor_cli_iface.get_pregs = local_pregs
processor_cli_iface.get_diff_regs = local_diff_regs
processor_cli_iface.get_pending_exception_string = local_pending_exception
processor_cli_iface.get_address_prefix = None
processor_cli_iface.translate_to_physical = None
SIM_register_interface(SIM_get_class('sample-risc-core'), 'processor_cli',
                       processor_cli_iface)


# Support for GUI register window
have_wx = False
if conf.sim.gui_mode != "not-supported":
    try:
        import winsome
    except ImportError:
        have_wx = False
    else:
        have_wx = hasattr(winsome, "win_main")

if have_wx:
    from winsome.win_registers import (install_register_class, ra, r, l,
                                       register_fields, dl, format_value,
                                       register_field)

    def risc_register_window(win, cpu):

        pages = [{'name' : 'Run-time registers', 'columns' : 4},
                 {'name' : 'Control registers', 'columns' : 1}]
        groups = [{'page' : 0, 'name' : 'Accumulators', 'col_span' : 2,
                   'regs' : [r( 'r0', 32, 'r zero'),
                             r( 'r1', 32, 'r one'),
                             r( 'r2', 32, 'r two'),
                             r( 'r3', 32, 'r three'),
                             r( 'r4', 32, 'r four'),
                             r( 'r5', 32, 'r five'),
                             r( 'r6', 32, 'r siz'),
                             r( 'r7', 32, 'r seven')],
                   'columns' : 2},
                  {'page' : 0, 'name' : 'Special purpose', 'col_span': 2,
                   'regs' : [r('r12', 32, 'r twelve'),
                             r('r13', 32, 'r thirteen'),
                             r('r14', 32, 'r fourteen'),
                             r('r15', 32, 'r fifteen')],
                   'columns' : 1},
                  {'page' : 0, 'name' : 'Address registers', 'col_span' : 4,
                   'regs' : [r( 'r8', 32, 'r eight'),
                             r( 'r9', 32, 'r nine'),
                             r('r10', 32, 'r ten'),
                             r('r11', 32, 'r eleven')],
                   'columns' : 4},
                  {'page' : 1, 'name' : 'Other', 'col_span' : 1,
                   'regs' : [r('msr', 20)],
                   'columns' : 1},
                  ]
        win.create_register_view(pages, *groups)

    install_register_class('sample-risc-core', risc_register_window)

opcode_info = opcode_length_info_t(min_alignment = 4,
                                   max_length = 4,
                                   avg_length = 4)

SIM_register_interface(SIM_get_class('sample-risc-core'), 'opcode_info',
                       opcode_info_interface_t(get_opcode_length_info
                                               = lambda cpu: opcode_info))
