import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_cx(cpu)
  paddr = 0
  cpu.pc = paddr
  cpu.gprs[2] = 6
  res = 0xa

  simics.SIM_write_phys_memory(cpu, paddr, 0x17020a00, 4)

  stest.expect_equal(cpu.pc, res)
  print "CALL works correctly"

  paddr = 0
  cpu.pc = paddr
  cpu.gprs[2] = 0
  res = 0xa

  simics.SIM_write_phys_memory(cpu, paddr, 0x17020a00, 4)

  stest.expect_equal(cpu.pc, 4)
  print "Is-statement works correctly"

test_cx(conf.chip0)
