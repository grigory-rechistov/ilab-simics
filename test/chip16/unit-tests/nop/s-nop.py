# This test checks NOP instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_nop_availability(cpu):
    paddr = 0
    cpu.pc = paddr
    # NOP
    chip16_write_phys_memory_BE(cpu, paddr, 0, 4)
    SIM_continue(1)
    stest.expect_equal(cpu.pc, paddr + 4)
    print "NOP: success"

test_nop_availability(conf.chip0)
