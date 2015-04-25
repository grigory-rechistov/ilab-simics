# This test checks SNP_[RX]_HHLL instruction

import stest
import time

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)
cli.run_command("enable-real-time-mode")

def test_snp_rx_hhll_availability(cpu):
        paddr = 0
        cpu.pc = paddr;

        # SNP_[RX]_HHLL
        # play tone from [R1]=750Hz for 0x64=100 ms
        freq = 7500
        cpu.gprs[1] = 0x1000
        simics.SIM_write_phys_memory(cpu, cpu.gprs[1], freq, 2)
        chip16_write_phys_memory_BE (cpu, paddr, 0x0D016400, 4)
        SIM_continue(1)
        cpu.core_enabled = False
        SIM_run_command("continue-seconds 0.1")
        cpu.core_enabled = True


        #check cpu things
        print "SNP_[RX]_HHLL_test-1: checking cpu.pc..."
        stest.expect_equal(cpu.pc, paddr + 4)
        print "SNP_[RX]_HHLL_test-1: cpu.pc is OK!"
        print ""

        # check attributes
        print "SNP_[RX]_HHLL_test-1: checking freq..."
        stest.expect_equal(conf.snd0.signal_freq, freq)
        print "SNP_[RX]_HHLL_test-1: freq is OK."
        print ""

        print "SNP_[RX]_HHLL_test-1: checking time limit..."
        stest.expect_equal(conf.snd0.limit, 0x64)
        print "SNP_[RX]_HHLL_test-1: time limit is OK."
        print ""

        print "SNP_[RX]_HHLL_test-1: All is OK!"

test_snp_rx_hhll_availability(conf.chip0)
