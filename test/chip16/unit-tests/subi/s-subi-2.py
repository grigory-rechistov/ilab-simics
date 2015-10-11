# This test checks SUBI instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        
#TEST1 on carry && overflow && negative flags
        paddr = 0x0
        cpu.pc = paddr
        cpu.flags = 0b11000110
        cpu.gprs[1] = 0x7000
        uimm = 0x8000
        
        res = (cpu.gprs[1] - uimm) % 0x10000
        chip16_write_phys_memory_BE(cpu, paddr, 0x50010080, 4)
        SIM_continue(1)
        
        print "Test1"
        stest.expect_equal(cpu.pc, paddr + 4)
        print "SUBI: (pc) success"
        stest.expect_equal(cpu.gprs[1], res)
        print "SUBI: (result) success"
        stest.expect_equal(cpu.flags, 0b11000010)
        print "SUBI: (cr&ovrfl_flag) success"
        
#TEST2 on negative && carry flag
        paddr = cpu.pc
        cpu.flags = 0b11000110
        cpu.gprs[1] = 0x1000
        uimm = 0x2000
        
        res = (cpu.gprs[1] - uimm) % 0x10000
        chip16_write_phys_memory_BE(cpu, paddr, 0x50010020, 4)
        SIM_continue(1)
        
        print "Test2"
        stest.expect_equal(cpu.pc, paddr + 4)
        print "SUBI: (pc) success"
        stest.expect_equal(cpu.gprs[1], res)
        print "SUBI: (result) success"
        stest.expect_equal(cpu.flags, 0b10000010)
        print "SUBI: (ng&cr_flag) success"

test_one_availability(conf.chip0)
