# This test checks NEG_X instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_neg_availability(cpu):
        cpu.pc = 0
        cpu.gprs[4] = 0xabcd
        res = (~0xabcd + 1) & 0xffff
        # NEG R4
        chip16_write_phys_memory_BE(cpu, 0, 0xe4040000, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, 4)
        stest.expect_equal(cpu.gprs[4], res)
        print "NEG_X: success"

        cpu.pc = 0
        cpu.gprs[5] = 0x0
        res = (~0x0 + 1) & 0xffff
        #NEG R5
        chip16_write_phys_memory_BE(cpu, 0, 0xe4050000, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.gprs[5], res)
        stest.expect_equal(cpu.flags, 0b00000100)
        print "NEG_X: zero success"

        cpu.pc = 0
        cpu.gprs[5] = 0x11
        res = (~0x11 + 1) & 0xffff
        #NEG R5
        chip16_write_phys_memory_BE(cpu, 0, 0xe4050000, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.gprs[5], res)
        stest.expect_equal(cpu.flags, 0b10000000)
        print "NEG_X: neg success"

test_neg_availability(conf.chip0)
