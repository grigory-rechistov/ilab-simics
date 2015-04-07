# This test checks POPALL instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_popall_availability(cpu):
        paddr = 0
        cpu.pc = paddr

        cpu_sp_save = cpu.sp


        cpu.sp = 0xfdf0 + 30

        for x in range(0, 16):
                SIM_write_phys_memory(cpu, cpu.sp - (2 * x), x + 10, 2)

        cpu.sp = 0xfdf0 + 32

        # POPALL
        chip16_write_phys_memory_BE(cpu, paddr, 0xc3000000, 4)
        SIM_continue(1)

        # check regs
        print "POPALL_test-1: checking pc..."
        stest.expect_equal(cpu.pc, paddr + 4)
        print "POPALL_test-1: pc OK."
        print " "

        print "POPALL_test-1: checking sp..."
        stest.expect_equal(cpu.sp, cpu_sp_save)
        print "POPALL_test-1: sp OK."
        print " "

        print "POPALL_test-1: checking gprs..."
        for x in range(0, 16):
                stest.expect_equal (cpu.gprs[x], x + 10)
        print "POPALL_test-1: gprs OK."
        print " "

        print "POPALL_test-1: regs OK."
        print " "

        print "POPALL_test-1: All is OK!"

test_popall_availability(conf.chip0)
