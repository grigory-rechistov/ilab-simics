# This test checks disasm function

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_disasm(cpu):

#------------------------------------------------------------------------------#
        print "ADDI_disasm_unit-test:"
#------------------------------------------------------------------------------#
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
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "MOV_disasm_unit-test:"
#------------------------------------------------------------------------------#
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
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "MULI_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print " "

        paddr = 0
        cpu.pc = paddr

        print "writing 'muli r5, 0xfeed'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0x9005edfe, 4)
        stest.expect_equal('p:0x0000  muli r5, 0xfeed', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'muli r5, 0xfeed' is OK."
        print " "

        print "writing 'muli r7, 0xded'..."
        paddr += 4
        cpu.pc = paddr
        simics.SIM_write_phys_memory(cpu, paddr, 0x9007ed0d, 4)
        stest.expect_equal('p:0x0004  muli r7, 0xded', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'muli r7, 0xded' is OK."
        print " "
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "NOP_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print " "

        paddr = 0
        cpu.pc = paddr

        print "writing 'nop'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0x00000000, 4)
        stest.expect_equal('p:0x0000  nop', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'nop' is OK."
        print " "
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "Unknown_instr_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print " "

        paddr = 0
        cpu.pc = paddr

        print "writing 'unknown: 0xffffffff' instruction..."
        simics.SIM_write_phys_memory(cpu, paddr, 0xffffffff, 4)
        stest.expect_equal('p:0x0000  unknown: 0xffffffff', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'unknown: 0xffffffff' instruction is OK."
        print " "
        print " "
#------------------------------------------------------------------------------#


        print "All is OK!"

test_disasm(conf.chip0)
