# This test checks ADDI instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_addi_availability(cpu):
        paddr = 0
        cpu.pc = paddr

        cpu.flags = 0

        cpu.gprs[7] = 0x0
        res = cpu.gprs[7] + 0x0
        res &= 0xffff

        # ADDI
        chip16_write_phys_memory_BE(cpu, paddr, 0x40070000, 4)
        SIM_continue(1)

        # check regs
        print "ADDI_test-2: checking regs..."
        stest.expect_equal(cpu.pc, paddr + 4)
        stest.expect_equal(cpu.gprs[7], res)
        print "ADDI_test-2: regs OK."
        print " "

        # check flags
        print "ADDI_test-2: checking flags..."
        stest.expect_equal(cpu.flags, 0b00000100)
        print "ADDI_test-2: flags OK."
        print " "

        print "ADDI_test-2: All is OK!"

test_addi_availability(conf.chip0)
