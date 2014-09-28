# This test checks NOTI instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):
    paddr = 0x4	
    cpu.pc = paddr
    cpu.gprs[4] = 0xffff
    HHLL	= 0xffff
    res         = ~HHLL

    # NOTI RX, RY
    simics.SIM_write_phys_memory(cpu, paddr, 0xE004FFFF, 4)
    SIM_continue(1)
    stest.expect_equal(cpu.pc, paddr + 4)
    stest.expect_equal(cpu.gprs[4] & 0xffff, 0)
    stest.expect_equal(cpu.flags[1] & 0xffff, 1)
    print "NOTI: success"


test_one_availability(conf.chip0)
