import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_call(cpu)
  paddr = 0
  cpu.pc = paddr
  cpu.gprs[2] = 6

  simics.SIM_write_phys_memory(cpu, paddr, 0x18020000, 4)

  stest.expect_equal(cpu.pc, cpu.gprs[2])


test_call(conf.chip0)
