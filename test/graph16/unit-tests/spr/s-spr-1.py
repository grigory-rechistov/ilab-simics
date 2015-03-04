# This test checks SPR instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):
        graph0_addr = 0xfff6

        # SPR 0xAABB
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0301, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xAABB, 2)

        stest.expect_equal(conf.graph0.spriteh, 0xAA)
        stest.expect_equal(conf.graph0.spritew, 0xBB)
        print "SPR(1): success"

        # SPR 0xCCDD
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0301, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xCCDD, 2)

        stest.expect_equal(conf.graph0.spriteh, 0xCC)
        stest.expect_equal(conf.graph0.spritew, 0xDD)
        print "SPR(2): success"

test_one_availability(conf.chip0)