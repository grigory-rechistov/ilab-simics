# This test checks FLIP instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):
        graph0_addr = 0xfff6

        # FLIP (0, 0)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0401, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0000, 2)

        stest.expect_equal(conf.graph0.hflip, 0)
        stest.expect_equal(conf.graph0.vflip, 0)
        print "FLIP(1): success"

        # FLIP (1, 0)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0401, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x2000, 2)

        stest.expect_equal(conf.graph0.hflip, 1)
        stest.expect_equal(conf.graph0.vflip, 0)
        print "FLIP(2): success"

test_one_availability(conf.chip0)