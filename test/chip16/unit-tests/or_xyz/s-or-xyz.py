# This test checks OR_XYZ instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_or_availability(cpu):
        cpu.pc = 0
        cpu.flags = 0

        cpu.gprs[1] = 0x1000
        cpu.gprs[2] = 0x0101
        res = 0x1000 | 0x0101
        # OR R1, R2, R3
        chip16_write_phys_memory_BE(cpu, 0, 0x72210A00, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, 4)
        stest.expect_equal(cpu.gprs[0xA], res)
        print "OR_XYZ: success"

        cpu.pc = 0
        cpu.flags = 0
        cpu.gprs[1] = 0x0
        cpu.gprs[2] = 0x0
        # OR R1, R2, R3
        chip16_write_phys_memory_BE(cpu, 0, 0x72210A00, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, 4)
        stest.expect_equal(cpu.flags, 0b00000100)
        print "OR_XYZ: zero success"

        cpu.pc = 0
        cpu.flags = 0
        cpu.gprs[1] = 0x0f00
        cpu.gprs[2] = 0xf000
        res = 0x0f00 | 0xf000
        # OR R1, R2, R3
        chip16_write_phys_memory_BE(cpu, 0, 0x72210A00, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, 4)
        stest.expect_equal(cpu.gprs[0xA], res)
        stest.expect_equal(cpu.flags, 0b10000000)
        print "OR_XYZ: neg success"

test_or_availability(conf.chip0)
