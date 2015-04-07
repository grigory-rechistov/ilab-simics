# This test checks NOTI instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        paddr = 0x0
        cpu.pc = paddr
        cpu.gprs[4] = 0xffff
        HHLL	= 0xffff
        res     = ~HHLL

        # NOTI RX, RY
        chip16_write_phys_memory_BE(cpu, paddr, 0xE004FFFF, 4)
        SIM_continue(1)
        
        stest.expect_equal(cpu.pc, paddr + 4)
        print "NOTI: (pc) success"
        
        stest.expect_equal(cpu.gprs[4] & 0xffff, res & 0xffff)
        print "NOTI: (result) success"
        
        stest.expect_equal(cpu.flags, 0b00000100)
        print "NOTI: (flags) success"

test_one_availability(conf.chip0)
