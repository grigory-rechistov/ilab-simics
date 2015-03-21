# This test checks DRW instruction (MUST BE IMPROVED)

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        graph0_addr = 0xfff6

        # SPR 0xAABB
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0301, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0202, 2)

        simics.SIM_write_phys_memory(cpu, 0x00, 0x0101010101010101, 8)

        # DRW (1, 2, 0xAABB)
        SIM_write_phys_memory(cpu, graph0_addr, 0x0003, 2)


        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x000F, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x000F, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0000, 2)

test_one_availability(conf.chip0)