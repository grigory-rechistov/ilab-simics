# This Software is part of Wind River Simics. The rights to copy, distribute,
# modify, or otherwise make use of this Software may be licensed only
# pursuant to the terms of an applicable Wind River license agreement.
# 
# Copyright 2010-2014 Intel Corporation

from cli import *

def info(obj):
    return [(None,
             [("Interrupt Device", obj.irq_dev)])]

def status(obj):
    return [(None,
             [('Reference', obj.regs_reference),
              ('Counter start time', obj.regs_counter_start_time),
              ('Counter start value', obj.regs_counter_start_value)])]

new_info_command("videotimer", info)
new_status_command("videotimer", status)

