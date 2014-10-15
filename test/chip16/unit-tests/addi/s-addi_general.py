# This test checks ADDI instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_addi_availability(cpu):
        paddr = 0
        cpu.pc = paddr

        cpu.flags = 0

        cpu.gprs[7] = 0xaa0
        res = cpu.gprs[7] + 0x1c
        res &= 0xffff

        # ADDI
        simics.SIM_write_phys_memory(cpu, paddr, 0x40071c00, 4)
        SIM_continue(1)

        # check regs
        print "ADDI_test-1: checking regs..."
        stest.expect_equal(cpu.pc, paddr + 4)
        stest.expect_equal(cpu.gprs[7], res)
        print "ADDI_test-1: regs OK."
        print " "

        # check flags
        print "ADDI_test-1: checking flags..."
        stest.expect_equal(cpu.flags, 0)
        print "ADDI_test-1: flags OK."
        print " "

        print "ADDI_test-1: All is OK!"

test_addi_availability(conf.chip0)
