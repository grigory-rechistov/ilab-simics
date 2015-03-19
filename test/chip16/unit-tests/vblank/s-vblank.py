# This test checks VBLANK instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_instruction(cpu):
    paddr = 0
    cpu.pc = paddr
    # VBLANK
    chip16_write_phys_memory_BE(cpu, paddr,   0x00000002, 4)
    # NOP
    chip16_write_phys_memory_BE(cpu, paddr+4, 0x00000000, 4)
    SIM_continue(1)
    stest.expect_equal(cpu.pc, paddr + 4)
    SIM_run_command("sc 1")
    stest.expect_equal(cpu.pc, paddr + 4)
    cpu.ports.signal.VBLANK.signal_raise()
    cpu.ports.signal.VBLANK.signal_lower()
    SIM_continue(1)
    stest.expect_equal(cpu.pc, paddr + 8)
    print "VBLANK: success"

test_instruction(conf.chip0)
