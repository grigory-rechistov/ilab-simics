# This test checks DRW instruction. It draws 8x8 pixels smile

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        graph0_addr = 0xfff6
        smile_addr  = 0x00
        smile_width = 8

        # SPR (8 ,8)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0301, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0808, 2)

        simics.SIM_write_phys_memory(cpu, smile_addr                    , 0x33333333, smile_width/2)
        simics.SIM_write_phys_memory(cpu, smile_addr + smile_width/2 * 1, 0xc33cc33c, smile_width/2)
        simics.SIM_write_phys_memory(cpu, smile_addr + smile_width/2 * 2, 0xc33cc33c, smile_width/2)
        simics.SIM_write_phys_memory(cpu, smile_addr + smile_width/2 * 3, 0x33333333, smile_width/2)
        simics.SIM_write_phys_memory(cpu, smile_addr + smile_width/2 * 4, 0x33333333, smile_width/2)
        simics.SIM_write_phys_memory(cpu, smile_addr + smile_width/2 * 5, 0xc333333c, smile_width/2)
        simics.SIM_write_phys_memory(cpu, smile_addr + smile_width/2 * 6, 0x33cccc33, smile_width/2)
        simics.SIM_write_phys_memory(cpu, smile_addr + smile_width/2 * 7, 0x33333333, smile_width/2)


        # DRW (0x009c, 0x00e4 , 0) (Drawing smile in the center of window)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0003, 2)


        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x009c, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0074, 2)
        simics.SIM_write_phys_memory(cpu, graph0_addr, 0x0000, 2)

test_one_availability(conf.chip0)