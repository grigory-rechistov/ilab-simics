# This test checks MULI instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_muli_availability(cpu):
        paddr = 0
        cpu.pc = paddr
        
        cpu.flags = 0

        cpu.gprs[7] = 0xab
        res = cpu.gprs[7] * (0xffff)
        res &= 0xffff

        # MULI
        simics.SIM_write_phys_memory(cpu, paddr, 0x9007ffff, 4)
        SIM_continue(1)
        
        # check regs
        print "MULI_1: checking regs..."
        stest.expect_equal(cpu.pc, paddr + 4)
        stest.expect_equal(cpu.gprs[7], res)
        print "MULI_1: regs OK."

        # check flags
        print "MULI_1: checking flags..."
        stest.expect_equal(cpu.flags, 0b10000010) # NEG & CARRY flag
        print "MULI_1: flags OK: NEG & CARRY is set."
        
        print "MULI_1: All is OK!"
        
test_muli_availability(conf.chip0)
