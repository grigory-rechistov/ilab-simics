import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_cmpi(cpu)
  paddr = 0
  cpu.pc = paddr
  cpu.flags = 0
  cpu.gprs[6] = 0xa

  simics.SIM_write_phys_memory(cpu, paddr, 0x53060d00, 4)

  stest.expect.equal(cpu.flags, 0x10000000)
  print "neg flag is ok"

  paddr = 0
  cpu.pc = paddr
  cpu.flags = 0
  cpu.gprs[6] = 0xd

  simics.SIM_write_phys_memory(cpu, paddr, 0x53060d00, 4)

  stest.expect.equal(cpu.flags, 0x00000100)
  print "zero flag is ok"

  paddr = 0
  cpu.pc = paddr
  cpu.flags = 0
  cpu.gprs[6] = 0xff

  simics.SIM_write_phys_memory(cpu, paddr, 0x5306ffff, 4)

  stest.expect.equal(cpu.flags, 0x01000000)
  print "overflw flag is ok"


test_cmpi(conf.chip0)
