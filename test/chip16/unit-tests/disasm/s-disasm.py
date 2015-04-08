# This test checks disasm function

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_disasm(cpu):

#------------------------------------------------------------------------------#
        print "NOP_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'nop'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x00000000, 4)
        stest.expect_equal('p:0x0000  nop', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'nop' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "SND0_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'snd0'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x09000000, 4)
        stest.expect_equal('p:0x0000  snd0', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'snd0' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "SND1_HHLL_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'snd1 0x1234'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x0A003412, 4)
        stest.expect_equal('p:0x0000  snd1 0x1234', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'snd1 0x1234' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "SND2_HHLL_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'snd2 0x1234'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x0B003412, 4)
        stest.expect_equal('p:0x0000  snd2 0x1234', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'snd2 0x1234' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "SND3_HHLL_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'snd3 0x1234'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x0C003412, 4)
        stest.expect_equal('p:0x0000  snd3 0x1234', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'snd3 0x1234' is OK."
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
        chip16_write_phys_memory_BE(cpu, paddr, 0x2100f4fd, 4)
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
        chip16_write_phys_memory_BE(cpu, paddr, 0x24750000, 4)
        stest.expect_equal('p:0x0000  mov r5, r7', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'mov r5, r7' is OK."

        print "writing 'mov r15, r0'..."
        paddr += 4
        cpu.pc = paddr
        chip16_write_phys_memory_BE(cpu, paddr, 0x240f0000, 4)
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
        chip16_write_phys_memory_BE(cpu, paddr, 0x31750000, 4)
        stest.expect_equal('p:0x0000  stm r5, r7', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'stm r5, r7' is OK."

        print "writing 'stm r15, r0'..."
        paddr += 4
        cpu.pc = paddr
        chip16_write_phys_memory_BE(cpu, paddr, 0x310f0000, 4)
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
        chip16_write_phys_memory_BE(cpu, paddr, 0x4005ed0d, 4)
        stest.expect_equal('p:0x0000  addi r5, 0xded', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'addi r5, 0xded' is OK."

        print "writing 'addi r7, 0xfeed'..."
        paddr += 4
        cpu.pc = paddr
        chip16_write_phys_memory_BE(cpu, paddr, 0x4007edfe, 4)
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
        chip16_write_phys_memory_BE(cpu, paddr, 0x9005edfe, 4)
        stest.expect_equal('p:0x0000  muli r5, 0xfeed', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'muli r5, 0xfeed' is OK."

        print "writing 'muli r7, 0xded'..."
        paddr += 4
        cpu.pc = paddr
        chip16_write_phys_memory_BE(cpu, paddr, 0x9007ed0d, 4)
        stest.expect_equal('p:0x0004  muli r7, 0xded', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'muli r7, 0xded' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "AND_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print " "

        paddr = 0
        cpu.pc = paddr

        print "writing 'and r4, r5, r6'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x62540600, 4)
        stest.expect_equal('p:0x0000  and r4, r5, r6', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'and r4, r5, r6' is OK."
        print " "

        print "writing 'and r15, r0, r5'..."
        paddr += 4
        cpu.pc = paddr
        chip16_write_phys_memory_BE(cpu, paddr, 0x620f0500, 4)
        stest.expect_equal('p:0x0004  and r15, r0, r5', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'and r15, r0, r5' is OK."
        print " "
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "NEGI_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print " "

        paddr = 0
        cpu.pc = paddr

        print "writing 'negi r10, 0xdead'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0xE30aadde, 4)
        stest.expect_equal('p:0x0000  negi r10, 0xdead', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'negi r10, 0xdead' is OK."
        print " "

        print "writing 'negi r0, 0x0'..."
        paddr += 4
        cpu.pc = paddr
        chip16_write_phys_memory_BE(cpu, paddr, 0xE3000000, 4)
        stest.expect_equal('p:0x0004  negi r0, 0x00', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'negi r0, 0x0' is OK."
        print " "
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "NEG_XY_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print " "

        paddr = 0
        cpu.pc = paddr

        print "writing 'neg r10, r0'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0xE50a0000, 4)
        stest.expect_equal('p:0x0000  neg r10, r0', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'neg r10, r0' is OK."
        print " "

        print "writing 'neg r15, r8'..."
        paddr += 4
        cpu.pc = paddr
        chip16_write_phys_memory_BE(cpu, paddr, 0xE58f0000, 4)
        stest.expect_equal('p:0x0004  neg r15, r8', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'neg r15, r8' is OK."
        print " "
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "RET_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print " "

        paddr = 0
        cpu.pc = paddr

        print "writing 'ret'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x15000000, 4)
        stest.expect_equal('p:0x0000  ret', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'ret' is OK."
        print " "
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "CALL_HHLL_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print " "

        paddr = 0
        cpu.pc = paddr

        print "writing 'call 0xfdf4'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x1400f4fd, 4)
        stest.expect_equal('p:0x0000  call 0xfdf4', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'call 0xfdf4' is OK."
        print " "
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "JMP_X_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print " "

        paddr = 0
        cpu.pc = paddr

        print "writing 'jmp r10'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x160a0000, 4)
        stest.expect_equal('p:0x0000  jmp r10', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'jmp r10' is OK."
        print " "
        print " "

        print "writing 'jmp r0'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x16000000, 4)
        stest.expect_equal('p:0x0000  jmp r0', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'jmp r0' is OK."
        print " "
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "POPALL_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'popall'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0xc3000000, 4)
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
        chip16_write_phys_memory_BE(cpu, paddr, 0xA1210000, 4)
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
        chip16_write_phys_memory_BE(cpu, paddr, 0xA2210300, 4)
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
        chip16_write_phys_memory_BE(cpu, paddr, 0x81210000, 4)
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
        chip16_write_phys_memory_BE(cpu, paddr, 0xA601ed0d, 4)
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
        chip16_write_phys_memory_BE(cpu, paddr, 0xE001ed0d, 4)
        stest.expect_equal('p:0x0000  noti r1, 0xded', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'noti r1, 0xded' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


#------------------------------------------------------------------------------#
        print "POP_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'pop r1'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0xC1010000, 4)
        stest.expect_equal('p:0x0000  pop r1', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'pop r1' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "NOT_X_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'not r1'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0xE1010000, 4)
        stest.expect_equal('p:0x0000  not r1', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'not r1' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "NEG_X_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'neg r2'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0xE4020000, 4)
        stest.expect_equal('p:0x0000  neg r2', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'neg r2' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "SUB_XY_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'sub r1, r2'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x51210000, 4)
        stest.expect_equal('p:0x0000  sub r1, r2', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'sub r1, r2' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "OR_XY_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'or r1, r2'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x71210000, 4)
        stest.expect_equal('p:0x0000  or r1, r2', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'or r1, r2' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "TSTI_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'tsti r1, 0xdead'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x6301adde, 4)
        stest.expect_equal('p:0x0000  tsti r1, 0xdead', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'tsti r1, 0xdead' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "TST_XY_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'tst r1, r2'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x64210000, 4)
        stest.expect_equal('p:0x0000  tst r1, r2', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'tst r1, r2' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "ADD_XYZ_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'add r1, r2, r3'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x42210300, 4)
        stest.expect_equal('p:0x0000  add r1, r2, r3', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'add r1, r2, r3' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "CALL_X_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'call r1'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x18010000, 4)
        stest.expect_equal('p:0x0000  call r1', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'call r1' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "Cx_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'Cx 0x3, 0xdead'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x1703adde, 4)
        stest.expect_equal('p:0x0000  Cx 0x3, 0xdead', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'Cx 0x3, 0xdead' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "CMPI_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'cmpi r1, 0xdead'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x5301adde, 4)
        stest.expect_equal('p:0x0000  cmpi r1, 0xdead', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'cmpi r1, 0xdead' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "LDI_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'ldi r1, 0xeeee'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x2001eeee, 4)
        stest.expect_equal('p:0x0000  ldi r1, 0xeeee', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'ldi r1, 0xeeee' is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
        print "STM_X_disasm_unit-test:"
#------------------------------------------------------------------------------#
        print "{"

        paddr = 0
        cpu.pc = paddr

        print "writing 'stm r1, 0xeeee'..."
        chip16_write_phys_memory_BE(cpu, paddr, 0x3001eeee, 4)
        stest.expect_equal('p:0x0000  stm r1, 0xeeee', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'stm r1, 0xeeee' is OK."
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
        chip16_write_phys_memory_BE(cpu, paddr, 0xffffffff, 4)
        stest.expect_equal('p:0x0000  [illegal instruction] instr = 0xffffffff', conf.chip0.iface.processor_cli.get_disassembly("p", conf.chip0.pc, False, None)[1])
        print "'unknown: 0xffffffff' instruction is OK."
        print "}"
        print " "
#------------------------------------------------------------------------------#


        print "All is OK!"

test_disasm(conf.chip0)
