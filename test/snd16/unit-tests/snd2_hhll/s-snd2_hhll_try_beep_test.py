# This test checks SND1_HHLL instruction

import stest
import time

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_snd1_hhll_availability(cpu):
        paddr = 0
        cpu.pc = paddr;

        # SND1_HHLL
        # play 1000Hz tone for 0x0064 ms
        chip16_write_phys_memory_BE(cpu, paddr, 0x0B006400, 4)
        SIM_continue(1)

        #check cpu things
        print "SND2_HHLL_test-1: checking cpu.pc..."
        stest.expect_equal(cpu.pc, paddr + 4)
        print "SND2_HHLL_test-1: cpu.pc is OK!"
        print ""

        # check attributes
        print "SND2_HHLL_test-1: checking freq..."
        stest.expect_equal(conf.snd0.signal_freq, 1000)
        print "SND2_HHLL_test-1: freq is OK."
        print ""

        print "SND2_HHLL_test-1: checking time limit..."
        stest.expect_equal(conf.snd0.limit, 0x64)
        print "SND2_HHLL_test-1: time limit is OK."
        print ""

        print "SND1_HHLL_test-1: All is OK!"

test_snd1_hhll_availability(conf.chip0)
