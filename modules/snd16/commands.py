# This Software is part of Wind River Simics. The rights to copy, distribute,
# modify, or otherwise make use of this Software may be licensed only
# pursuant to the terms of an applicable Wind River license agreement.
# 
# Copyright 2010-2014 Intel Corporation

import cli
import os
import simics

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

#
# ------------------------ wav-file-start -----------------------
#

def wav_file_start_cmd(obj, output_file, overwrite):
    try:
    	obj.out_file = output_file
    except simics.SimExc_IllegalValue:
        if (os.path.exists(output_file) and overwrite):
            os.remove(output_file)
            obj.out_file = output_file
        else:
            print "File with this name already exists, use -overwrite flag or change name \
                   and after it set wav_enable again\n"

    

cli.new_command("wav-file-start", wav_file_start_cmd,
                [cli.arg(cli.str_t, "output_file"), cli.arg(cli.flag_t, "-overwrite")],
                alias = "",
                type  = "snd16 commands",
                short = "records sound to wav file",
                cls = "snd16",
                doc = "Creates wav file with given pathname. By default it returns an error \
                if the file with this name already exists, but it could be changed with overwrite flag")

#
# ------------------------ wav-file-stop -----------------------
#
                
def wav_file_stop_cmd(obj):
    obj.out_file = ""
    
cli.new_command("wav-file-stop", wav_file_stop_cmd,
                [],
                alias = "",
                type  = "snd16 commands",
                short = "stops current sound recording",
                cls = "snd16",
                doc = "Stops current sound recording")
