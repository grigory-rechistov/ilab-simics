# This test checks AND_XY instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):

        paddr = 0x0
        cpu.pc = paddr
        cpu.gprs[1] = 0x1000
        cpu.gprs[2] = 0x1001

        # AND RX,RY
        chip16_write_phys_memory_BE(cpu, paddr, 0x612100, 3)
        SIM_continue(1)

        stest.expect_equal(cpu.pc, paddr + 4)
        print "AND: (pc) success"

        stest.expect_equal(cpu.gprs[1], 0x1000 & cpu.gprs[2])
        print "AND: (result) success"

        cpu.gprs[4] = 0x0
        cpu.gprs[5] = 0x1001

        chip16_write_phys_memory_BE(cpu, cpu.pc, 0x615400, 3)
        SIM_continue(1)
        stest.expect_equal(cpu.flags, 0b00000100)
        print "AND: (ZERO) success"

        cpu.gprs[7] = ~0x100;
        cpu.gprs[8] = ~0x123;

        chip16_write_phys_memory_BE(cpu, cpu.pc, 0x618700, 3)
        SIM_continue(1)
        stest.expect_equal(cpu.gprs[7],~0x100 & cpu.gprs[8])
        stest.expect_equal(cpu.flags, 0b10000000)
        print "AND: (NEG) success"

test_one_availability(conf.chip0)
