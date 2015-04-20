# This test checks TST_XY instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        cpu.pc = 0
        cpu.flags = 0
        cpu.gprs[1] = 0x1000
        cpu.gprs[2] = 0x0101
        # TST R1, R2
        chip16_write_phys_memory_BE(cpu, 0, 0x64210000, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, 4)
        stest.expect_equal(cpu.flags, 0b00000100)
        print "TST_XY: zero success"

        cpu.pc = 0
        cpu.flags = 0
        cpu.gprs[1] = 0xff00
        cpu.gprs[2] = 0xf000
        # TST R1, R2
        chip16_write_phys_memory_BE(cpu, 0, 0x64210000, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, 4)
        stest.expect_equal(cpu.flags, 0b10000000)
        print "TST_XY: neg success"

test_one_availability(conf.chip0)
