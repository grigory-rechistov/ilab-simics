import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_ldm(cpu) :
    paddr = 0
    cpu.pc = paddr
    cpu.gprs[1] = addr = 0x1234
    data = 0xdada
    chip16_write_phys_memory_BE(cpu, addr, data, 2)

    # LDM R7, R1
    chip16_write_phys_memory_BE(cpu, paddr, 0x23170000, 4)
    SIM_continue(1)

    stest.expect_equal(cpu.pc, paddr + 4)
    stest.expect_equal(cpu.gprs[7], data)
    print "LDM_XY: success"

test_ldm(conf.chip0)
