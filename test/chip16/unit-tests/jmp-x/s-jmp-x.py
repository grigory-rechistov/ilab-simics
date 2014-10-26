# This test checks JMP_X instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):

        paddr = 0x4
        cpu.pc = paddr
        cpu.gprs[10] = 0x1004

        # JMP RX
        simics.SIM_write_phys_memory(cpu, paddr, 0x160a0000, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.pc, cpu.gprs[10])
        print "JMP_X: (pc) success"

test_one_availability(conf.chip0)
