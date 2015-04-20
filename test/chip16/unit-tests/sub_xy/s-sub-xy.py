import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_sub(cpu) :
        cpu.flags = 0
        cpu.pc = 0
        cpu.gprs[6] = 0xa
        cpu.gprs[2] = 0xa
        res = 0xa - 0xa
        # SUB R6, R2
        chip16_write_phys_memory_BE(cpu, 0, 0x51260000, 4)
        SIM_continue(1);
        stest.expect_equal(cpu.pc, 4)
        stest.expect_equal(cpu.gprs[6], res)
        stest.expect_equal(cpu.flags, 0b00000100)
        print "SUB_XY: sub is ok"
        print "SUB_XY: zero is ok"

        cpu.flags = 0
        cpu.pc = 0
        cpu.gprs[1] = 0xa
        cpu.gprs[2] = 0xd
        res = (0xa - 0xd) & 0xffff
        # SUB R6, R2
        chip16_write_phys_memory_BE(cpu, 0, 0x51210000, 4)
        SIM_continue(1);
        stest.expect_equal(cpu.gprs[1], res)
        stest.expect_equal(cpu.flags, 0b10000010)
        print "SUB_XY: neg & carry is ok"

        cpu.flags = 0
        cpu.pc = 0
        cpu.gprs[1] = 0x8001
        cpu.gprs[2] = 0x2
        res = (0x8001 - 0x2) & 0xffff
        # SUB R6, R2
        chip16_write_phys_memory_BE(cpu, 0, 0x51210000, 4)
        SIM_continue(1);
        stest.expect_equal(cpu.gprs[1], res)
        stest.expect_equal(cpu.flags, 0b01000000)
        print "SUB_XY: ovf is ok"

test_sub(conf.chip0)
