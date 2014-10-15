# This test checks ADDI instruction disasm function

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_addi_availability(cpu):
        print "ADDI_disasm_unit-test:"
        print " "

        paddr = 0
        cpu.pc = paddr

        print "writing 'addi r5, 0xded'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0x4005ed0d, 4)
        stest.expect_equal('p:0x0000  addi r5, 0xded', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'addi r5, 0xded' is OK."
        print " "

        print "writing 'addi r7, 0xfeed'..."
        paddr += 4
        cpu.pc = paddr
        simics.SIM_write_phys_memory(cpu, paddr, 0x4007edfe, 4)
        stest.expect_equal('p:0x0004  addi r7, 0xfeed', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'addi r7, 0xfeed' is OK."
        print " "

        print "All is OK!"

test_addi_availability(conf.chip0)
