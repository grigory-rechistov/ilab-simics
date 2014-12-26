# This test checks DIV instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):
        graph0_addr = 0xfff6

        # DRW (1, 2, 0xAABB)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0003, 2)

        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0001, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0002, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0xAABB, 2)

test_one_availability(conf.chip0)