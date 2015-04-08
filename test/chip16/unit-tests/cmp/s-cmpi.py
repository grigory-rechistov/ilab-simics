import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_cmpi(cpu) :
    cpu.flags = 0
    cpu.gprs[6] = 0xa
    # CMPI R6, 0xd
    chip16_write_phys_memory_BE(cpu, 0, 0x53060d00, 4)
    SIM_continue(1);
    stest.expect_equal(cpu.pc, 4)
    stest.expect_equal(cpu.flags, 0b10000010)
    print "CMPI: NEG & CARRY is ok"

    cpu.flags = 0
    cpu.gprs[6] = 0xd
    cpu.pc = 0
    # CMPI R6, 0xd
    chip16_write_phys_memory_BE(cpu, 0, 0x53060d00, 4)
    SIM_continue(1);
    stest.expect_equal(cpu.flags, 0b00000100)
    print "CMPI: ZERO is ok"

    cpu.flags = 0
    cpu.gprs[6] = 0x8001
    cpu.pc = 0
    # CMPI R6, 0x2
    chip16_write_phys_memory_BE(cpu, 0, 0x53060200, 4)
    SIM_continue(1);
    stest.expect_equal(cpu.flags, 0b01000000)
    print "CMPI: OVF is ok"

test_cmpi(conf.chip0)
