# This test checks STM_X instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_stm_xy_availability(cpu):
        paddr = 0
        cpu.pc = paddr
        cpu.gprs[5] = 0x11
        HHLL = 0xaabb

        # STM R5 0xaabb
        chip16_write_phys_memory_BE(cpu, paddr, 0x3005bbaa, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.pc, paddr + 4)
        print "STM_X: pc is OK."
        stest.expect_equal(simics.SIM_read_phys_memory(cpu, HHLL, 2), cpu.gprs[5])
        print "STM_X: memory is OK."

test_stm_xy_availability(conf.chip0)
