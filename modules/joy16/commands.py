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

#
# ------------------------ add-log -----------------------
#

def add_log_cmd(obj, str):
    try:
        obj.add_log = str
    except Exception, ex:
        print "Error adding log string: %s" % ex.message


cli.new_command("add-log", add_log_cmd,
                [cli.arg(cli.str_t, "log-string", "?", "default text")],
                alias = "",
                type  = "joy16 commands",
                short = "add a text line to the device log",
                namespace = "joy16",
                doc = """
Add a line of text to the device log. Use the 'log' command to view the log
after creating a log-buffer using 'log-size'.
""")

#
# ---------------- simple method ------------------------
#

def simple_method_cmd(obj, arg):
    obj.iface.sample.simple_method(arg)

cli.new_command("simple-method", simple_method_cmd,
                [cli.arg(cli.int_t, "arg")],
                alias = "",
                type  = "joy16 commands",
                short = "simple example method",
                namespace = "joy16",
                doc = "Simple method used as a sample. Prints the argument.")
