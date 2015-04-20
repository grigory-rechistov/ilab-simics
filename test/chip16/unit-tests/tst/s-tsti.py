# This test checks TSTI instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        cpu.pc = 0
        cpu.flags = 0
        cpu.gprs[2] = 0x0101
        # TSTI R2, 0x1000
        chip16_write_phys_memory_BE(cpu, 0, 0x63020010, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, 4)
        stest.expect_equal(cpu.flags, 0b00000100)
        print "TSTI: zero success"

        cpu.pc = 0
        cpu.flags = 0
        cpu.gprs[2] = 0xf000
        # TSTI R2, 0xff00
        chip16_write_phys_memory_BE(cpu, 0, 0x630200ff, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, 4)
        stest.expect_equal(cpu.flags, 0b10000000)
        print "TSTI: neg success"

test_one_availability(conf.chip0)
