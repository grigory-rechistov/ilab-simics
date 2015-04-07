# This test checks BGC instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        graph0_addr = 0xfff6

        # BGC 0x0A
        SIM_write_phys_memory(cpu, graph0_addr, 0x0201, 2)
        SIM_write_phys_memory(cpu, graph0_addr, 0xA000, 2)

        stest.expect_equal(conf.graph0.bg, 0xA)
        print "BGC(1): success"

        # BGC 0x0B
        SIM_write_phys_memory(cpu, graph0_addr, 0x0201, 2)
        SIM_write_phys_memory(cpu, graph0_addr, 0xB000, 2)

        stest.expect_equal(conf.graph0.bg, 0xB)
        print "BGC(2): success"

test_one_availability(conf.chip0)