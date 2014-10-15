# This test checks 'unknown instruction' disasm function

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_unknown_instr_availability(cpu):
        print "Unknown_instr_disasm_unit-test:"
        print " "

        paddr = 0
        cpu.pc = paddr

        print "writing 'unknown: 0xffffffff' instruction..."
        simics.SIM_write_phys_memory(cpu, paddr, 0xffffffff, 4)
        stest.expect_equal('p:0x0000  unknown: 0xffffffff', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'unknown: 0xffffffff' instruction is OK."
        print " "

        print "All is OK!"

test_unknown_instr_availability(conf.chip0)
