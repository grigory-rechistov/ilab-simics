# This test checks loading binary data from raw files (without header)

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)
raw_c16 = "%s/test/chip16/unit-tests/binary/file3.c16" % conf.sim.workspace
raw_bin = "%s/test/chip16/unit-tests/binary/file4.bin" % conf.sim.workspace
cpu = current_processor()

cli.run_command("chip16-load-binary %s" % raw_c16)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x0), 0x10)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x1), 0x00)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x2), 0x0a)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x3), 0x00)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x4), 0x14)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x5), 0x00)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x6), 0x03)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x7), 0x00)
print "Load-binary: (raw_c16) success"

simics.SIM_write_phys_memory(cpu, 0, 0, 8)
cli.run_command("chip16-load-binary %s" % raw_bin)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x0), 0x10)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x1), 0x00)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x2), 0x0a)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x3), 0x00)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x4), 0x14)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x5), 0x00)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x6), 0x03)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x7), 0x00)
print "Load-binary: (raw_bin) success"
