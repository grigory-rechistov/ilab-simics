# This test checks REMI instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):
        paddr = 0x0
        cpu.pc = paddr
        cpu.gprs[4] = 50
        HHLL	= 5
        res = cpu.gprs[4] % HHLL

        # REMI RX, HHLL
        simics.SIM_write_phys_memory(cpu, paddr, 0xA6040500, 4)
        SIM_continue(1)
        
        stest.expect_equal(cpu.pc, paddr + 4)
        print "REMI: (pc) success"
        
        stest.expect_equal(cpu.gprs[4], res)
        print "REMI: (result) success"
        
        stest.expect_equal(cpu.flags, 0b00000100)
        print "REMI: (flags) success"

test_one_availability(conf.chip0)
