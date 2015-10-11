# This test checks SUBI instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        
#TEST1 on negative flag
        paddr = 0x0
        cpu.pc = paddr
        cpu.flags = 0b11000110
        cpu.gprs[1] = 0x8111
        uimm = 0x0010
        
        res = cpu.gprs[1] - uimm
        chip16_write_phys_memory_BE(cpu, paddr, 0x50011000, 4)
        SIM_continue(1)
        
        print "Test1"
        stest.expect_equal(cpu.pc, paddr + 4)
        print "SUBI: (pc) success"
        stest.expect_equal(cpu.gprs[1], res)
        print "SUBI: (result) success"
        stest.expect_equal(cpu.flags, 0b10000000)
        print "SUBI: (neg_flag) success"
        
#TEST2 on zero flag
        paddr = cpu.pc
        cpu.flags = 0b11000110
        cpu.gprs[1] = 0x1101
        uimm = 0x1101
        
        res = cpu.gprs[1] - uimm
        chip16_write_phys_memory_BE(cpu, paddr, 0x50010111, 4)
        SIM_continue(1)
        
        print "Test2"
        stest.expect_equal(cpu.pc, paddr + 4)
        print "SUBI: (pc) success"
        stest.expect_equal(cpu.gprs[1], res)
        print "SUBI: (result) success"
        stest.expect_equal(cpu.flags, 0b0000100)
        print "SUBI: (zero_flag) success"

#TEST3 on carry flag
        paddr = cpu.pc
        cpu.flags = 0b11000110
        cpu.gprs[1] = 0x8100
        uimm = 0x0101
        
        res = cpu.gprs[1] - uimm
        chip16_write_phys_memory_BE(cpu, paddr, 0x50010101, 4)
        SIM_continue(1)

        print "Test3"        
        stest.expect_equal(cpu.pc, paddr + 4)
        print "SUBI: (pc) success"
        stest.expect_equal(cpu.gprs[1], res)
        print "SUBI: (result) success"
        stest.expect_equal(cpu.flags, 0b01000000)
        print "SUBI: (carry_flag) success"

test_one_availability(conf.chip0)
