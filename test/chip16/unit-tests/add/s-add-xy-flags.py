import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def testAdd(cpu) :
    cpu.gprs[1] = 0xffff
    cpu.gprs[2] = 0xffff
    res = (0xffff + 0xffff) & 0xffff
    # ADD r1, r2
    chip16_write_phys_memory_BE(cpu, 0, 0x41210000, 4)
    SIM_continue(1);

    stest.expect_equal(cpu.gprs[1], res)
    stest.expect_equal(cpu.flags, 0b10000010)
    print "ADD_XY: carry & neg are OK"

    cpu.gprs[2] = 0xffff
    cpu.gprs[3] = 0x1
    res = 0
    cpu.pc = 0
    cpu.flags = 0
    # ADD r3, r2
    chip16_write_phys_memory_BE(cpu, 0, 0x41230000, 4)
    SIM_continue(1);

    stest.expect_equal(cpu.gprs[3], res)
    stest.expect_equal(cpu.flags, 0b00000110)
    print "ADD_XY: carry & zero are OK"

    cpu.gprs[2] = 0x7fff
    cpu.gprs[3] = 0x2
    res = (0x7fff + 0x2) & 0xffff
    cpu.pc = 0
    cpu.flags = 0
    # ADD r3, r2
    chip16_write_phys_memory_BE(cpu, 0, 0x41230000, 4)
    SIM_continue(1);

    stest.expect_equal(cpu.gprs[3], res)
    stest.expect_equal(cpu.flags, 0b11000000)
    print "ADD_XYZ: ovf & neg are OK"

testAdd(conf.chip0)
