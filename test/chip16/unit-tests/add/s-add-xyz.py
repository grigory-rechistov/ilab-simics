import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def testAdd(cpu) :
    paddr = 0
    cpu.pc = paddr
    cpu.gprs[2] = 2
    cpu.gprs[3] = 3
    cpu.gprs[8] = 666
    res = cpu.gprs[2] + cpu.gprs[3]

    chip16_write_phys_memory_BE(cpu, paddr, 0x42230800, 4)
    SIM_continue(1);

    stest.expect_equal(cpu.pc, paddr + 4)
    stest.expect_equal(cpu.gprs[8], res)
    stest.expect_equal(cpu.flags, 0)
    print "ADD_XYZ: regs & pc are OK"

testAdd(conf.chip0)
