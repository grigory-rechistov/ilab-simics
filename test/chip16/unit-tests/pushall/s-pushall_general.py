# This test checks PUSHALL instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_pushall_availability(cpu):
        paddr = 0
        cpu.pc = paddr

        cpu_sp_save = cpu.sp + 32

        for x in range(0, 16):
                cpu.gprs[x] = x

        # PUSHALL
        chip16_write_phys_memory_BE(cpu, paddr, 0xc2000000, 4)
        SIM_continue(1)

        # check regs
        print "PUSHALL_test-1: checking pc..."
        stest.expect_equal(cpu.pc, paddr + 4)
        print "PUSHALL_test-1: pc OK."
        print " "

        print "PUSHALL_test-1: checking sp..."
        stest.expect_equal(cpu.sp, cpu_sp_save)
        print "PUSHALL_test-1: sp OK."
        print " "

        print "PUSHALL_test-1: checking gprs..."
        for x in range(0, 16):
                stest.expect_equal (cpu.gprs[x], 
                        SIM_read_phys_memory(cpu, cpu.sp - (2 * (x + 1)), 2) & 65535)
        print "PUSHALL_test-1: gprs OK."
        print " "

        print "PUSHALL_test-1: All is OK!"

test_pushall_availability(conf.chip0)
