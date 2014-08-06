# This test checks NOP instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_nop_availability(cpu):
    paddr = 0
    cpu.pc = paddr
    # NOP
    simics.SIM_write_phys_memory(cpu, paddr, 0, 4)
    SIM_continue(1)
    stest.expect_equal(cpu.pc, paddr + 4)
    print "NOP: success"

test_nop_availability(conf.chip0)
