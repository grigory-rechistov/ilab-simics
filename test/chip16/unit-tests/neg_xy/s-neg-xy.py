# This test checks NEG_XY instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):

        paddr = 0x0
        cpu.pc = paddr
        cpu.gprs[1] = 0xbaaad
        cpu.gprs[2] = 0x1001

        # NEG RX,RY
        simics.SIM_write_phys_memory(cpu, paddr, 0xE5210000, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.pc, paddr + 4)
        print "NEG_XY: (pc) success"

        stest.expect_equal(cpu.gprs[1] & 0xffff, (~cpu.gprs[2] + 1) & 0xffff)
        print "NEG_XY: (result) success"

        stest.expect_equal(cpu.flags, 0b10000000)
        print "NEG_XY: (NEG) success"

        cpu.gprs[3] = 0xbaaad
        cpu.gprs[4] = 0x0

        simics.SIM_write_phys_memory(cpu, cpu.pc, 0xE5430000, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.flags, 0b00000100)
        print "NEG_XY: (ZERO) success"

test_one_availability(conf.chip0)
