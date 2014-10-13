import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_ldi(cpu)
  paddr = 0
  cpu.pc = paddr
  cpu.gprs[2] = 6
  res = 0xd


  simics.SIM_write_phys_memory(cpu, paddr, 0x20070d00, 4)

  stest.expect_equal(cpu.gprs[7], res)

test_ldi(conf.chip0)
