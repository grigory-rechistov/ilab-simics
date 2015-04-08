# This test checks JMP_HHLL instruction
import stest
cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        cpu.pc = 0
        HHLL = 0xaa10
        # JMP 0xaa10
        chip16_write_phys_memory_BE(cpu, 0, 0x100010aa, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, HHLL)
        print "JMP_HHLL success"

test_one_availability(conf.chip0)
