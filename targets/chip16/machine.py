# This Software is part of Wind River Simics. The rights to copy, distribute,
# modify, or otherwise make use of this Software may be licensed only
# pursuant to the terms of an applicable Wind River license agreement.
# 
# Copyright 2010-2014 Intel Corporation

name_prefix = cli.simenv.host_name
if name_prefix != "":
    name_prefix = name_prefix + "_"

sample_risc0 = pre_conf_object(name_prefix + "sample_risc0", "sample-risc")
sample_risc0.queue = sample_risc0
sample_risc0.freq_mhz =  1.0

ram_image0 = pre_conf_object(name_prefix + "ram_image0", "image")
ram_image0.queue = sample_risc0
ram_image0.size = 0x800000

ram_image1 = pre_conf_object(name_prefix + "ram_image1", "image")
ram_image1.queue = sample_risc0
ram_image1.size = 0x800000

ram0 = pre_conf_object(name_prefix + "ram0", "ram")
ram0.image = ram_image0

ram1 = pre_conf_object(name_prefix + "ram1", "ram")
ram1.image = ram_image1

test0 = pre_conf_object(name_prefix + "test0", "memory-space")
test0.queue = sample_risc0
test0.map = [[0x0, ram1, 0, 0, 0x800000]]

sample_dev0 = pre_conf_object(name_prefix + "sample_dev0", "sample_device_dml")
sample_dev0.queue = sample_risc0

sample_dev1 = pre_conf_object(name_prefix + "sample_dev1", "sample_event_device")
sample_dev1.queue = sample_risc0

phys_mem0 = pre_conf_object(name_prefix + "phys_mem0", "memory-space")
phys_mem0.queue = sample_risc0
phys_mem0.map = [[     0x0, ram0,                 0, 0, 0x800000],
                [ 0xa00000, [sample_dev0, "reg"], 0, 0,     0x10],
                [ 0xb00000, [sample_dev1, "reg"], 0, 0,     0x10],
                [0x1000000, test0,                0, 0, 0x800000]]

ctx0 = pre_conf_object(name_prefix + "ctx0", "context")
ctx0.queue = sample_risc0

sample_core0 = pre_conf_object(name_prefix + "sample_core0", "sample-risc-core")
sample_core0.queue = sample_risc0
sample_core0.sample_risc = sample_risc0
sample_core0.physical_memory_space = phys_mem0
sample_core0.current_context = ctx0

sample_core1 = pre_conf_object(name_prefix + "sample_core1", "sample-risc-core")
sample_core1.queue = sample_risc0
sample_core1.sample_risc = sample_risc0
sample_core1.physical_memory_space = phys_mem0
sample_core1.current_context = ctx0

cosim_cell = pre_conf_object(name_prefix + "cosim_cell", "cell")
cosim_cell.current_processor = sample_core0
cosim_cell.current_step_obj = sample_risc0
cosim_cell.current_cycle_obj = sample_risc0
cosim_cell.scheduled_object = sample_risc0

sample_risc0.cell = cosim_cell
sample_risc0.current_risc_core = sample_core0

SIM_add_configuration([sample_risc0, ctx0, cosim_cell,
                       ram_image0, ram_image1, ram0, ram1, test0, phys_mem0,
                       sample_dev0, sample_dev1, sample_core0, sample_core1],
                      None)
