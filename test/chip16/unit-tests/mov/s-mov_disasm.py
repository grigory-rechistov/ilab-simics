# This test checks MOV instruction disasm function

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_mov_availability(cpu):
        print "MOV_disasm_unit-test:"
        print " "

        paddr = 0
        cpu.pc = paddr

        print "writing 'mov r5, r7'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0x24750000, 4)
        stest.expect_equal('p:0x0000  mov r5, r7', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'mov r5, r7' is OK."
        print " "

        print "writing 'mov r15, r0'..."
        paddr += 4
        cpu.pc = paddr
        simics.SIM_write_phys_memory(cpu, paddr, 0x240f0000, 4)
        stest.expect_equal('p:0x0004  mov r15, r0', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'mov r15, r0' is OK."
        print " "

        print "All is OK!"

test_mov_availability(conf.chip0)
