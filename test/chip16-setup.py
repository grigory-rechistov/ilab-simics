cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def chip16_write_phys_memory_BE(cpu, paddr, instr, length):
    rev_instr = 0
    for k in xrange(length):
        rev_instr += (instr >> ((length - k - 1) * 8) & 0xff) << k * 8

    simics.SIM_write_phys_memory(cpu, paddr, rev_instr, length)
