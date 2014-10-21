# This test checks disasm function

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_disasm(cpu):

#------------------------------------------------------------------------------#
        print "NOP_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'nop'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0x00000000, 4)
        stest.expect_equal('p:0x0000  nop', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'nop' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "LDI_SP_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'ldi sp, 0xfdf4'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0x2100f4fd, 4)
        stest.expect_equal('p:0x0000  ldi sp, 0xfdf4', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'ldi sp, 0xfdf4' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "MOV_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'mov r5, r7'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0x24750000, 4)
        stest.expect_equal('p:0x0000  mov r5, r7', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'mov r5, r7' is OK."

        print "writing 'mov r15, r0'..."
        paddr += 4
        cpu.pc = paddr
        simics.SIM_write_phys_memory(cpu, paddr, 0x240f0000, 4)
        stest.expect_equal('p:0x0004  mov r15, r0', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'mov r15, r0' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "STM_XY_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'stm r5, r7'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0x31750000, 4)
        stest.expect_equal('p:0x0000  stm r5, r7', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'stm r5, r7' is OK."

        print "writing 'stm r15, r0'..."
        paddr += 4
        cpu.pc = paddr
        simics.SIM_write_phys_memory(cpu, paddr, 0x310f0000, 4)
        stest.expect_equal('p:0x0004  stm r15, r0', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'stm r15, r0' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "ADDI_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'addi r5, 0xded'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0x4005ed0d, 4)
        stest.expect_equal('p:0x0000  addi r5, 0xded', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'addi r5, 0xded' is OK."

        print "writing 'addi r7, 0xfeed'..."
        paddr += 4
        cpu.pc = paddr
        simics.SIM_write_phys_memory(cpu, paddr, 0x4007edfe, 4)
        stest.expect_equal('p:0x0004  addi r7, 0xfeed', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'addi r7, 0xfeed' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "MULI_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'muli r5, 0xfeed'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0x9005edfe, 4)
        stest.expect_equal('p:0x0000  muli r5, 0xfeed', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'muli r5, 0xfeed' is OK."

        print "writing 'muli r7, 0xded'..."
        paddr += 4
        cpu.pc = paddr
        simics.SIM_write_phys_memory(cpu, paddr, 0x9007ed0d, 4)
        stest.expect_equal('p:0x0004  muli r7, 0xded', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'muli r7, 0xded' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "POPALL_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'popall'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0xc3000000, 4)
        stest.expect_equal('p:0x0000  popall', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'popall' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "DIV_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'div r1, r2'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0xA1210000, 4)
        stest.expect_equal('p:0x0000  div r1, r2', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'div r1, r2' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "DIV_XYZ_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'div r1, r2, r3'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0xA2210300, 4)
        stest.expect_equal('p:0x0000  div r1, r2, r3', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'div r1, r2, r3' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "XOR_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'xor r1, r2'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0x81210000, 4)
        stest.expect_equal('p:0x0000  xor r1, r2', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'xor r1, r2' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "REMI_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'remi r1, 0xded'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0xA601ed0d, 4)
        stest.expect_equal('p:0x0000  remi r1, 0xded', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'remi r1, 0xded' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "NOTI_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'noti r1, 0xded'..."
        simics.SIM_write_phys_memory(cpu, paddr, 0xE001ed0d, 4)
        stest.expect_equal('p:0x0000  noti r1, 0xded', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'noti r1, 0xded' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "Unknown_instr_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'unknown: 0xffffffff' instruction..."
        simics.SIM_write_phys_memory(cpu, paddr, 0xffffffff, 4)
        stest.expect_equal('p:0x0000  unknown: 0xffffffff', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'unknown: 0xffffffff' instruction is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


        print "All is OK!"

test_disasm(conf.chip0)
