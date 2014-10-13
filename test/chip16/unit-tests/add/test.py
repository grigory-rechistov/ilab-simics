import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_add(cpu)
  paddr = 0
  cpu.pc = paddr
  cpu.gprs[2] = 2
  cpu.gprs[3] = 3
  cpu.gprs[8] = 9
  res = cpu.gprs[2] * cpu.gprs[3]

  simics.SIM_write_phys_memory(cpu, paddr, 0x42230800, 4)

  stest.expect_equal(cpu.pc, paddr + 4)
  stest.expect_equal(cpu.gprs[7], res)

test_add(conf.chip0)
