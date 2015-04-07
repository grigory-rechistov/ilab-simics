# This test checks DRW instruction (MUST BE IMPROVED)

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        graph0_addr = 0xfff6

        # DRW (1, 2, 0xAABB)
        SIM_write_phys_memory(cpu, graph0_addr, 0x0003, 2)

        SIM_write_phys_memory(cpu, graph0_addr, 0x0001, 2)
        SIM_write_phys_memory(cpu, graph0_addr, 0x0002, 2)
        SIM_write_phys_memory(cpu, graph0_addr, 0xAABB, 2)

test_one_availability(conf.chip0)