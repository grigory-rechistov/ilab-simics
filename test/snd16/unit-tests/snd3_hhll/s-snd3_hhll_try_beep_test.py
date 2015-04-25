# This test checks SND3_HHLL instruction

import stest
import time

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)
cli.run_command("enable-real-time-mode")

def test_snd3_hhll_availability(cpu):
        paddr = 0
        cpu.pc = paddr;

        # SND3_HHLL
        # play 1500Hz tone for 0x0064 ms
        chip16_write_phys_memory_BE(cpu, paddr, 0x0C006400, 4)
        SIM_continue(1)
        cpu.core_enabled = False
        SIM_run_command("continue-seconds 0.1")
        cpu.core_enabled = True


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

        print "SND3_HHLL_test-1: All is OK!"

test_snd3_hhll_availability(conf.chip0)
