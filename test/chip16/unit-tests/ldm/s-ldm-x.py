import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_ldm(cpu) :
    paddr = 0
    cpu.pc = paddr
    hhll = 0x1234
    data = 0xdada
    chip16_write_phys_memory_BE(cpu, hhll, data, 2)

    # LDM R7, 0x1234
    chip16_write_phys_memory_BE(cpu, paddr, 0x22073412, 4)
    SIM_continue(1)

    stest.expect_equal(cpu.pc, paddr + 4)
    stest.expect_equal(cpu.gprs[7], data)
    print "LDM_X: success"

test_ldm(conf.chip0)
