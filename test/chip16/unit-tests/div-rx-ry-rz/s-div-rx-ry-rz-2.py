# This test checks DIV_RX_RY_RZ instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):
        paddr = 0x0
        cpu.pc = paddr
        
        cpu.gprs[4] = 501
        cpu.gprs[2] = 0
        cpu.gprs[1] = 0
        
        # DIV RX, RY, RZ
        simics.SIM_write_phys_memory(cpu, paddr, 0xA2240100, 4)
        SIM_continue(1)
        
        stest.expect_equal(cpu.pc, paddr + 4)
        print "DIV_RX_RY_RZ:(pc) success"
        
test_one_availability(conf.chip0)
