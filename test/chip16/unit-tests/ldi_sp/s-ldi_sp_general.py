# This test checks LDI_SP instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_ldi_sp_availability(cpu):
        paddr = 0
        cpu.pc = paddr

        res = 0xfdf2
        res &= 0xffff

        # LDI_SP
        simics.SIM_write_phys_memory(cpu, paddr, 0x2100f2fd, 4)
        SIM_continue(1)

        # check regs
        print "LDI_SP_test-1: checking regs..."
        stest.expect_equal(cpu.pc, paddr + 4)
        stest.expect_equal(cpu.sp, res)
        print "LDI_SP_test-1: regs OK."
        print " "

        print "LDI_SP_test-1: All is OK!"

test_ldi_sp_availability(conf.chip0)
