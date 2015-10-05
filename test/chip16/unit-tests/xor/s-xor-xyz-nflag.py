# This test checks XOR instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
		paddr = 0x0
		cpu.pc = paddr
		cpu.gprs[4] = 0xF111
		cpu.gprs[2] = 0x0101
		cpu.gprs[3] = 0xF010
		res = cpu.gprs[4] ^ cpu.gprs[2]

        # XOR RX, RY, RZ
		chip16_write_phys_memory_BE(cpu, paddr, 0x82240300, 4)
		SIM_continue(1)

		stest.expect_equal(cpu.pc, paddr + 4)
		print "XOR: (pc) success"

		stest.expect_equal(cpu.gprs[3], res)
		print "XOR: (result) success"

		stest.expect_equal(cpu.flags, 0b10000000)
		print "XOR: (flags) success"

test_one_availability(conf.chip0)
