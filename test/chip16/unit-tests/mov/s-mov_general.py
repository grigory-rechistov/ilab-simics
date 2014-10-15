# This test checks MOV instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_mov_availability(cpu):
        paddr = 0
        cpu.pc = paddr

#------------------------------------------------------------------------------#
        cpu.flags = 0

        cpu.gprs[5] = 0x0
        cpu.gprs[7] = 0xded

        # MOV
        simics.SIM_write_phys_memory(cpu, paddr, 0x24750000, 4)
        SIM_continue(1)

        # check regs
        print "MOV_test-1.1: checking regs..."
        stest.expect_equal(cpu.pc, paddr + 4)
        stest.expect_equal(cpu.gprs[5], cpu.gprs[7])
        print "MOV_test-1.1: regs OK."
        print ""

        # check flags
        print "MOV_test-1.1: checking flags..."
        stest.expect_equal(cpu.flags, 0)
        print "MOV_test-1.1: flags OK."
        print ""
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        paddr += 4

        cpu.flags = 0

        cpu.gprs[15] = 0xbed
        cpu.gprs[0] = 0xfeed

        # MOV
        simics.SIM_write_phys_memory(cpu, paddr, 0x240f0000, 4)
        SIM_continue(1)

        # check regs
        print "MOV_test-1.2: checking regs..."
        stest.expect_equal(cpu.pc, paddr + 4)
        stest.expect_equal(cpu.gprs[15], cpu.gprs[0])
        print "MOV_test-1.2: regs OK."
        print ""

        # check flags
        print "MOV_test-1.2: checking flags..."
        stest.expect_equal(cpu.flags, 0)
        print "MOV_test-1.2: flags OK."
        print ""
#------------------------------------------------------------------------------#

        print "MOV_test-1: All is OK!"

test_mov_availability(conf.chip0)
