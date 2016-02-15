# This Software is part of Wind River Simics. The rights to copy, distribute,
# modify, or otherwise make use of this Software may be licensed only
# pursuant to the terms of an applicable Wind River license agreement.
# 
# Copyright 2010-2014 Intel Corporation

import cli

#
# ------------------------ info -----------------------
#

def get_sample_info(obj):
    return []

cli.new_info_command('joy16', get_sample_info)

#
# ------------------------ status -----------------------
#

def get_sample_status(obj):
    return [(None,
             [("Attribute 'value'", obj.value)])]

cli.new_status_command('joy16', get_sample_status)
