# This test checks MULI disasm instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_muli_availability(cpu):
        print "MULI_disasm_unit-test:"
        print " "

        paddr = 0
        cpu.pc = paddr

        print "writing 'nop'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0x00000000, 4)
        stest.expect_equal('p:0x0000  nop', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'nop' is OK."
        print " "

        print "writing 'muli r5, 0xfeed'..."
        paddr += 4
        cpu.pc = paddr
        simics.SIM_write_phys_memory(cpu, paddr, 0x9005edfe, 4)
        stest.expect_equal('p:0x0004  muli r5, 0xfeed', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'muli r5, 0xfeed' is OK."
        print " "

        print "writing 'muli r7, 0xded'..."
        paddr += 4
        cpu.pc = paddr
        simics.SIM_write_phys_memory(cpu, paddr, 0x9007ed0d, 4)
        stest.expect_equal('p:0x0008  muli r7, 0xded', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'muli r7, 0xded' is OK."
        print " "

        print "writing 'unknown: 0xffffffff' instruction..."
        paddr += 4
        cpu.pc = paddr
        simics.SIM_write_phys_memory(cpu, paddr, 0xffffffff, 4)
        stest.expect_equal('p:0x000c  unknown: 0xffffffff', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'unknown: 0xffffffff' instruction is OK."
        print " "

        print "All is OK!"

test_muli_availability(conf.chip0)
