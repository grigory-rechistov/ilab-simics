# This test checks STM_XY instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_stm_xy_availability(cpu):
        paddr = 0
        cpu.pc = paddr

        # STM_XY_test_1
        cpu.gprs[5] = 0xabcd
        cpu.gprs[7] = 0x1234

        chip16_write_phys_memory_BE(cpu, paddr, 0x31750000, 4)
        SIM_continue(1)

        # check pc
        print "STM_XY_test-1.1: checking pc..."
        stest.expect_equal(cpu.pc, paddr + 4)
        print "STM_XY_test-1.1: pc is OK."

        # check memory
        print "STM_XY_test-1.1: checking memory..."
        stest.expect_equal(simics.SIM_read_phys_memory(cpu, cpu.gprs[7], 2), 0xabcd)
        print "STM_XY_test-1.1: memory is OK."
        print " "


        # STM_XY_test_1.2
        cpu.gprs[0x0] = 0xabc
        cpu.gprs[0xf] = 0x4321

        chip16_write_phys_memory_BE(cpu, paddr + 4, 0x31f00000, 4)
        SIM_continue(1)

        # check pc
        print "STM_XY_test-1.2: checking pc..."
        stest.expect_equal(cpu.pc, paddr + 8)
        print "STM_XY_test-1.2: pc is OK."

        # check memory
        print "STM_XY_test-1.2: checking memory..."
        stest.expect_equal(simics.SIM_read_phys_memory(cpu, cpu.gprs[0xf], 2), 0xabc)
        print "STM_XY_test-1.2: memory is OK."
        print " "


        print "STM_XY_test-1: All is OK!"

test_stm_xy_availability(conf.chip0)
