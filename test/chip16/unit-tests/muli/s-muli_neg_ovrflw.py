# This test checks MULI instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_muli_availability(cpu):
        paddr = 0
        cpu.pc = paddr

        cpu.flags = 0

        cpu.gprs[7] = 0x2
        res = cpu.gprs[7] * (0x7fff)

        # MULI
        simics.SIM_write_phys_memory(cpu, paddr, 0x9007ff7f, 4)
        SIM_continue(1)

        # check regs
        print "MULI_test-4: checking regs..."
        stest.expect_equal(cpu.pc, paddr + 4)
        stest.expect_equal(cpu.gprs[7], res)
        print "MULI_test-4: regs OK."

        # check flags
        print "MULI_test-4: checking flags..."
        stest.expect_equal(cpu.flags, 0b11000000) # NEG & OVRFLW flag
        print "MULI_test-4: flags OK: NEG & OVRFLW is set."

        print "MULI_test-4: All is OK!"

test_muli_availability(conf.chip0)
