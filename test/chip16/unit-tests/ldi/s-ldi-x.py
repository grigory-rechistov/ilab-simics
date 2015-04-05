import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_ldi(cpu) :
    paddr = 0
    cpu.pc = paddr
    res = 0xd

    chip16_write_phys_memory_BE(cpu, paddr, 0x20070d00, 4)
    SIM_continue(1)

    stest.expect_equal(cpu.pc, paddr + 4)
    stest.expect_equal(cpu.gprs[7], res)
    print "LDI_X: success"

test_ldi(conf.chip0)
