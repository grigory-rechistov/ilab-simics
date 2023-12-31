# This Software is part of Wind River Simics. The rights to copy, distribute,
# modify, or otherwise make use of this Software may be licensed only
# pursuant to the terms of an applicable Wind River license agreement.
#
# Copyright 2010-2014 Intel Corporation

name_prefix = "" # Why is it here?

chip0 = pre_conf_object(name_prefix + "chip0", "chip16")
chip0.queue = chip0
chip0.freq_mhz =  1.0

ram_image0 = pre_conf_object(name_prefix + "ram_image0", "image")
ram_image0.queue = chip0
ram_image0.size = 0x10000

video_image = pre_conf_object(name_prefix + "video_image", "image")
video_image.queue = chip0
video_image.size = (320 / 2) *240

ram0 = pre_conf_object(name_prefix + "ram0", "ram")
ram0.image = ram_image0

video_ram = pre_conf_object(name_prefix + "video_ram", "ram")
video_ram.image = video_image

snd0 = pre_conf_object(name_prefix + "snd0", "snd16")
snd0.queue = chip0

joy0 = pre_conf_object(name_prefix + "joy0", "joy16")
joy0.queue = chip0

timer0 = pre_conf_object(name_prefix + "timer0", "videotimer")
timer0.queue = chip0
timer0.irq_dev = joy0
timer0.regs_step = 1
timer0.regs_config = 3

#joy1 = pre_conf_object(name_prefix + "joy1", "joy16")
#joy1.queue = chip0

graph0 = pre_conf_object(name_prefix + "graph0", "graph16")
graph0.queue = chip0

phys_mem0 = pre_conf_object(name_prefix + "phys_mem0", "memory-space")
phys_mem0.queue = chip0
phys_mem0.map = [[0x0,    ram0,     0, 0, 0xfff0],
                [ 0xfff0, joy0,     0, 0, 0x2   ],
                [ 0xfff4, snd0,     0, 0, 0x2   ],
                [ 0xfff6, graph0,   0, 0, 0x2   ]]

video_mem = pre_conf_object(name_prefix + "video_mem", "memory-space")
video_mem.queue = chip0
video_mem.map = [[0x0,  video_ram, 0, 0, video_image.size]]

ctx0 = pre_conf_object(name_prefix + "ctx0", "context")
ctx0.queue = chip0

chip0.physical_memory_space = phys_mem0
chip0.current_context = ctx0

graph0.physical_memory_space = phys_mem0
graph0.video_memory_space = video_mem

cosim_cell = pre_conf_object(name_prefix + "cosim_cell", "cell")
cosim_cell.current_processor = chip0
cosim_cell.current_step_obj = chip0
cosim_cell.current_cycle_obj = chip0
cosim_cell.scheduled_object = chip0

chip0.cell = cosim_cell

SIM_add_configuration([chip0, ctx0, cosim_cell, ram_image0, video_image, ram0, video_ram, phys_mem0, joy0, snd0, graph0, timer0, video_mem],
                      None)

conf.timer0.regs_reference = 1000
