# This test checks SND1_HHLL instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)
cli.run_command("enable-real-time-mode")

def test_snd1_hhll_availability(cpu, snd):
        paddr = 0
        cpu.pc = paddr
        snd.out_file = "logs/snd1.wav"
        snd.wav_enable = 1

        # SND1_HHLL
        # play 500Hz tone for 0x64=100 ms
        chip16_write_phys_memory_BE(cpu, paddr, 0x0A00E803, 4)
        SIM_continue(1)
        cpu.core_enabled = False
        SIM_run_command("continue-seconds 1")
        cpu.core_enabled = True


        #check cpu things
        print "SND1_HHLL_test-1: checking cpu.pc..."
        stest.expect_equal(cpu.pc, paddr + 4)
        print "SND1_HHLL_test-1: cpu.pc is OK!"
        print ""

        # check attributes
        print "SND1_HHLL_test-1: checking freq..."
        stest.expect_equal(conf.snd0.signal_freq, 500)
        print "SND1_HHLL_test-1: freq is OK."
        print ""

        print "SND1_HHLL_test-1: checking time limit..."
        stest.expect_equal(conf.snd0.limit, 0x3E8)
        print "SND1_HHLL_test-1: time limit is OK."
        print ""

        print "SND1_HHLL_test-1: All is OK!"

test_snd1_hhll_availability(conf.chip0, conf.snd0)
