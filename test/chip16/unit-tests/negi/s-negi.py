# This test checks NEGI instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):

        paddr = 0x4
        cpu.pc = paddr
        cpu.gprs[1] = 0xbaad
        HHLL = 0x1011

        # NEGI RX, HHLL
        simics.SIM_write_phys_memory(cpu, paddr, 0xE3011110, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.pc, paddr + 4)
        print "NEGI: (pc) success"

        stest.expect_equal(cpu.gprs[1] & 0xffff, (~HHLL + 1) & 0xffff)
        print "NEGI: (result) success"

        cpu.gprs[2] = 0x0

        simics.SIM_write_phys_memory(cpu, cpu.pc, 0xE3020000, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.flags, 0b00000100)
        print "NEGI: (ZERO) success"

        cpu.gprs[3] = 0x100;

        simics.SIM_write_phys_memory(cpu, cpu.pc, 0xE3031011, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.flags, 0b10000000)
        print "NEGI: (NEG) success"

test_one_availability(conf.chip0)
