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

cli.new_info_command('snd16', get_sample_info)

#
# ------------------------ status -----------------------
#

def get_sample_status(obj):
    return [(None,
             ["No status yet"]
             )]

cli.new_status_command('snd16', get_sample_status)



def wav_file_start_cmd(obj, output_file):
    #try:
        obj.out_file = output_file
    #except Exception, ex:
        #
        


cli.new_command("wav-file-start", wav_file_start_cmd,
                [cli.arg(cli.str_t, "output_file")],
                alias = "",
                type  = "snd16 commands",
                short = "simple example method",
                namespace = "snd16",
                doc = "Simple method used as a sample. Prints the argument.")
                
def wav_file_stop_cmd(obj):
    obj.out_file = ""
    
cli.new_command("wav-file-stop", wav_file_stop_cmd,
                [],
                alias = "",
                type  = "snd16 commands",
                short = "simple example method",
                namespace = "snd16",
                doc = "Simple method used as a sample. Prints the argument.")
