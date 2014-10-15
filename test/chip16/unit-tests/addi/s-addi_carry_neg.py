# This test checks ADDI instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_addi_availability(cpu):
        paddr = 0
        cpu.pc = paddr

        cpu.flags = 0

        cpu.gprs[7] = 0xffff
        res = cpu.gprs[7] + 0xffff
        res &= 0xffff

        # ADDI
        simics.SIM_write_phys_memory(cpu, paddr, 0x4007ffff, 4)
        SIM_continue(1)

        # check regs
        print "ADDI_test-3: checking regs..."
        stest.expect_equal(cpu.pc, paddr + 4)
        stest.expect_equal(cpu.gprs[7], res)
        print "ADDI_test-3: regs OK."
        print " "

        # check flags
        print "ADDI_test-3: checking flags..."
        stest.expect_equal(cpu.flags, 0b10000010)
        print "ADDI_test-3: flags OK."
        print " "

        print "ADDI_test-3: All is OK!"

test_addi_availability(conf.chip0)
