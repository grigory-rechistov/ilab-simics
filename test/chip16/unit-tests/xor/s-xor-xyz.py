# This test checks XOR instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        paddr = 0x00000000
        cpu.pc = paddr
        """Set wrong value for checking of working"""
        cpu.flags = 0b10000100
        cpu.gprs[4] = 0x1111
        cpu.gprs[2] = 0x0101
        res = cpu.gprs[4] ^ cpu.gprs[2]

        # XOR RX, RY, RZ
        chip16_write_phys_memory_BE(cpu, paddr, 0x82240500, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.pc, paddr + 4)
        print "XOR: (pc) success"

        stest.expect_equal(cpu.gprs[5], res)
        print "XOR: (result) success"

        stest.expect_equal(cpu.flags, 0b00000000)
        print "XOR: (flags) success"

test_one_availability(conf.chip0)
