# This test checks PAL instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):
        graph0_addr = 0xfff6

        # PAL (0xAABBCC for all)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0118, 2)

        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xAABB, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xCCAA, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xBBCC, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xAABB, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xCCAA, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xBBCC, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xAABB, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xCCAA, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xBBCC, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xAABB, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xCCAA, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xBBCC, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xAABB, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xCCAA, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xBBCC, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xAABB, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xCCAA, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xBBCC, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xAABB, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xCCAA, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xBBCC, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xAABB, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xCCAA, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xBBCC, 2)

        i = 0
        for i in range(0, 16 * 3, 3):
                stest.expect_equal(conf.graph0.palette[i + 0], 0xAA)
                stest.expect_equal(conf.graph0.palette[i + 1], 0xBB)
                stest.expect_equal(conf.graph0.palette[i + 2], 0xCC)
        print "PAL: success"

test_one_availability(conf.chip0)