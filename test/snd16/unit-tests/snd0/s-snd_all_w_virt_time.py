# This test checks SND1_HHLL instruction

import stest
import time

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)
cli.run_command("enable-real-time-mode")

def test_snd1_hhll_availability(cpu):
        paddr = 0
        cpu.pc = paddr;

        # SND1_HHLL
        simics.SIM_write_phys_memory(cpu, paddr, 0x0A006400, 4)
        paddr += 4;

        simics.SIM_write_phys_memory(cpu, paddr, 0x0B006400, 4)
        paddr += 4;

        simics.SIM_write_phys_memory(cpu, paddr, 0x0C006400, 4)
        paddr += 4;

        SIM_continue (1);
        cpu.core_enabled = False
        SIM_run_command("continue-seconds 0.1")
        cpu.core_enabled = True

        SIM_continue (1);
        cpu.core_enabled = False
        SIM_run_command("continue-seconds 0.1")
        cpu.core_enabled = True

        SIM_continue (1);
        cpu.core_enabled = False
        SIM_run_command("continue-seconds 0.1")
        cpu.core_enabled = True


        print "SND1_HHLL_test-1: All is OK!"

test_snd1_hhll_availability(conf.chip0)
