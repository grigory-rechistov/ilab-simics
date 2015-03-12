# This test checks overall fuctionality of chip16-load-binary func

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

correct_crc = "%s/test/chip16/unit-tests/binary/file1.c16" % conf.sim.workspace
cpu = current_processor()

cli.run_command("chip16-load-binary %s" % correct_crc)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x0), 0x10)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x1), 0x00)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x2), 0x0a)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x3), 0x00)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x4), 0x14)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x5), 0x00)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x6), 0x03)
stest.expect_equal(simics.SIM_read_byte(cpu.physical_memory_space, 0x7), 0x00)
print "Load-binary: (correct_crc) success"
