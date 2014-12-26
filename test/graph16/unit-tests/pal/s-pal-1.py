# This test checks DIV instruction

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

test_one_availability(conf.chip0)