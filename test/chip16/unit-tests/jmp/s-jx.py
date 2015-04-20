# This test checks Conditional jump: Jx instruction
import stest
cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        cpu.pc = 0
        HHLL = 0x10aa
        # Jx 0x2, 0x10aa
        cpu.flags = 0b10000000
        chip16_write_phys_memory_BE(cpu, 0, 0x1202aa10, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, HHLL)
        print "Jx (success)"
        print "Jump if Negative (success)"

        cpu.pc = 0
        HHLL = 0x1111
        # Jx 0xc, 0x1111
        cpu.flags = 0
        chip16_write_phys_memory_BE(cpu, 0, 0x120c1111, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, HHLL)
        print "Jump if Signed greater than or equal (success)"

        cpu.pc = 0
        HHLL = 0x1001
        # Jx 0x8, 0x1001
        cpu.flags = 0
        chip16_write_phys_memory_BE(cpu, 0, 0x12090110, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.pc, 0)
        print "Not jump (success)"

test_one_availability(conf.chip0)
