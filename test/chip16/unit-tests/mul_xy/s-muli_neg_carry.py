# This test checks MUL_XY instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_mul_xy_availability(cpu):
        paddr = 0
        cpu.pc = paddr
        cpu.flags = 0

        cpu.gprs[7] = 0xab
        cpu.gprs[6] = 0xffff
        res = cpu.gprs[7] * cpu.gprs[6]
        res &= cpu.gprs[6]

        # MUL_XY
        chip16_write_phys_memory_BE(cpu, paddr, 0x91670000, 4)
        SIM_continue(1)

        # check regs
        print "MUL_XY_test-3: checking regs..."
        stest.expect_equal(cpu.pc, paddr + 4)
        stest.expect_equal(cpu.gprs[7], res)
        print "MUL_XY_test-3: regs OK."

        # check flags
        print "MUL_XY_test-3: checking flags..."
        stest.expect_equal(cpu.flags, 0b10000010) # NEG & CARRY flag
        print "MUL_XY_test-3: flags OK: NEG & CARRY is set."

        print "MUL_XY_test-3: All is OK!"

test_mul_xy_availability(conf.chip0)
