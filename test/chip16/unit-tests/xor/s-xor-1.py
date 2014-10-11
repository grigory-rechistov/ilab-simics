# This test checks XOR instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):
        paddr = 0x0
        cpu.pc = paddr
        cpu.gprs[4] = 0x1111
        cpu.gprs[2] = 0x0101
        res = cpu.gprs[4] ^ cpu.gprs[2]

        # XOR RX, RY
        simics.SIM_write_phys_memory(cpu, paddr, 0x81240000, 4)
        SIM_continue(1)
        
        stest.expect_equal(cpu.pc, paddr + 4)
        print "XOR: (pc) success"
        
        stest.expect_equal(cpu.gprs[4], res)
        print "XOR: (result) success"
        
        stest.expect_equal(cpu.flags, 0b00000000)
        print "XOR: (flags) success"


test_one_availability(conf.chip0)
