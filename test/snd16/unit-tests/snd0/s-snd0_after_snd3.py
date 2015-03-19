# This test checks SND0 instruction

import stest
import time

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)
cli.run_command("enable-real-time-mode")

def test_snd0_availability(cpu):
        paddr = 0
        cpu.pc = paddr;

        # SND3_HHLL (for very long time)
        chip16_write_phys_memory_BE(cpu, paddr, 0x0C00FFFF, 4)
        paddr += 4;

        # SND0
        chip16_write_phys_memory_BE(cpu, paddr, 0x09000000, 4)
        paddr += 4;

        SIM_continue (1);
        cpu.core_enabled = False
        SIM_run_command("continue-seconds 0.1")
        cpu.core_enabled = True

        mute_sound_found = False
        for item in cpu.time_queue:
                if item[1] == "Mute sound":
                        mute_sound_found = True

        stest.expect_true(mute_sound_found)


        SIM_continue (1);

        for item in cpu.time_queue:
                stest.expect_true(item[1] != "Mute sound")

        print "SND0_test-1: All is OK!"

test_snd0_availability(conf.chip0)
