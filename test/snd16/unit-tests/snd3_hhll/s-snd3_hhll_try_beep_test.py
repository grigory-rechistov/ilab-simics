# This test checks SND1_HHLL instruction

import stest
import time

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_snd1_hhll_availability(cpu):
        paddr = 0
        cpu.pc = paddr;

        # SND1_HHLL
        # play 1500Hz tone for 0x0064 ms
        simics.SIM_write_phys_memory(cpu, paddr, 0x0C006400, 4)
        SIM_continue(1)

        #check cpu things
        print "SND3_HHLL_test-1: checking cpu.pc..."
        stest.expect_equal(cpu.pc, paddr + 4)
        print "SND3_HHLL_test-1: cpu.pc is OK!"
        print ""

        # check attributes
        print "SND3_HHLL_test-1: checking freq..."
        stest.expect_equal(conf.snd0.signal_freq, 1500)
        print "SND3_HHLL_test-1: freq is OK."
        print ""

        print "SND3_HHLL_test-1: checking time limit..."
        stest.expect_equal(conf.snd0.limit, 0x64)
        print "SND3_HHLL_test-1: time limit is OK."
        print ""

        print "SND1_HHLL_test-1: All is OK!"

test_snd1_hhll_availability(conf.chip0)
