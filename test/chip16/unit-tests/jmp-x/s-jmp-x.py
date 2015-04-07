# This test checks JMP_X instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):

        paddr = 0x4
        cpu.pc = paddr
        cpu.gprs[10] = 0x1004

        # JMP RX
        chip16_write_phys_memory_BE(cpu, paddr, 0x160a0000, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.pc, cpu.gprs[10])
        print "JMP_X: (pc) success"

test_one_availability(conf.chip0)
