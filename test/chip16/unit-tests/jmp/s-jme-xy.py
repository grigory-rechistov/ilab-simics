# This test checks JME_XY_HHLL instruction
import stest
cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        cpu.pc = 0
        HHLL = 0x10aa
        cpu.gprs[1] = 0xa
        cpu.gprs[2] = 0xa
        # JME r1, r2, 0x10aa
        chip16_write_phys_memory_BE(cpu, 0, 0x1321aa10, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, HHLL)

        cpu.pc = 0
        cpu.gprs[1] = 0xb
        cpu.gprs[2] = 0xa
        # JME r1, r2, 0x10aa
        chip16_write_phys_memory_BE(cpu, 0, 0x132110aa, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, 4)

        print "JME_XY_HHLL success"

test_one_availability(conf.chip0)
