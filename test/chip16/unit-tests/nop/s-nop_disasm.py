# This test checks NOP instruction disasm function

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_nop_availability(cpu):
        print "NOP_disasm_unit-test:"
        print " "

        paddr = 0
        cpu.pc = paddr

        print "writing 'nop'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0x00000000, 4)
        stest.expect_equal('p:0x0000  nop', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'nop' is OK."
        print " "

        print "All is OK!"

test_nop_availability(conf.chip0)
