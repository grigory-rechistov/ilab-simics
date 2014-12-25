# This test checks DIV instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):
        graph0_addr = 0xfff6

        # BGC 0x0A
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0201, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xA000, 2)

        stest.expect_equal(conf.graph0.bg, 0xA)
        print "BGC(1): success"

        # BGC 0x0B
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0201, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xB000, 2)

        stest.expect_equal(conf.graph0.bg, 0xB)
        print "BGC(2): success"

test_one_availability(conf.chip0)