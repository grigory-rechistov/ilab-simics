# This test checks ADDI instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_addi_availability(cpu):
        paddr = 0
        cpu.pc = paddr

#------------------------------------------------------------------------------#
# operands are negative, result is positive
#------------------------------------------------------------------------------#
        cpu.flags = 0

        cpu.gprs[7] = 0x8000
        res = cpu.gprs[7] + 0x8000
        res &= 0xffff

        # ADDI
        chip16_write_phys_memory_BE(cpu, paddr, 0x40070080, 4)
        SIM_continue(1)

        # check regs
        print "ADDI_test-4.1: checking regs..."
        stest.expect_equal(cpu.pc, paddr + 4)
        stest.expect_equal(cpu.gprs[7], res)
        print "ADDI_test-4.1: regs OK."
        print " "

        # check flags
        print "ADDI_test-4.1: checking flags..."
        stest.expect_equal(cpu.flags, 0b01000110)
        print "ADDI_test-4.1: flags OK."
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
# operands are positive, result is negative
#------------------------------------------------------------------------------#
        paddr += 4

        cpu.flags = 0

        cpu.gprs[8] = 0x4001
        res = cpu.gprs[8] + 0x4000
        res &= 0xffff

        # ADDI
        chip16_write_phys_memory_BE(cpu, paddr, 0x40080040, 4)
        SIM_continue(1)

        # check regs
        print "ADDI_test-4.2: checking regs..."
        stest.expect_equal(cpu.pc, paddr + 4)
        stest.expect_equal(cpu.gprs[8], res)
        print "ADDI_test-4.2: regs OK."
        print " "

        # check flags
        print "ADDI_test-4.2: checking flags..."
        stest.expect_equal(cpu.flags, 0b11000000)
        print "ADDI_test-4.2: flags OK."
        print " "
#------------------------------------------------------------------------------#

        print "ADDI_test-4: All is OK!"

test_addi_availability(conf.chip0)
