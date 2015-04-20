# This test checks JMC_HHLL instruction
import stest
cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        cpu.pc = 0
        cpu.flags = 0b00000010
        HHLL = 0x10aa
        # JMC 0x10aa
        chip16_write_phys_memory_BE(cpu, 0, 0x1100aa10, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, HHLL)
        print "JMC_HHLL success"

test_one_availability(conf.chip0)
