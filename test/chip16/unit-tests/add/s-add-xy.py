import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def testAdd(cpu) :
    paddr = 0
    cpu.pc = paddr
    cpu.gprs[1] = 10
    cpu.gprs[5] = 9
    res = cpu.gprs[1] + cpu.gprs[5]

    # ADD R1, R5
    chip16_write_phys_memory_BE(cpu, paddr, 0x41510000, 4)
    SIM_continue(1);

    stest.expect_equal(cpu.pc, paddr + 4)
    stest.expect_equal(cpu.gprs[1], res)
    stest.expect_equal(cpu.flags, 0)
    print "ADD_XY: regs & pc are OK"

testAdd(conf.chip0)
