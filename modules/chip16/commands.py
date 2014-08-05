# This Software is part of Wind River Simics. The rights to copy, distribute,
# modify, or otherwise make use of this Software may be licensed only
# pursuant to the terms of an applicable Wind River license agreement.
# 
# Copyright 2010-2014 Intel Corporation

from cli import *
from simics import *
import cli_impl
import sim_commands

def get_chip16_info(obj):
    return []

def get_chip16_status(obj):
    return []

new_info_command("chip16", get_chip16_info)
new_status_command("chip16", get_chip16_status)

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
SIM_register_interface(SIM_get_class('chip16'), 'processor_cli', processor_cli_iface)


opcode_info = opcode_length_info_t(min_alignment = 4,
                                   max_length = 4,
                                   avg_length = 4)

SIM_register_interface(SIM_get_class('chip16'), 'opcode_info',
                       opcode_info_interface_t(get_opcode_length_info
                                               = lambda cpu: opcode_info))
