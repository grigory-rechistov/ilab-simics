# This test checks Conditional Call: Cx instruction
import stest
cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        paddr = 0x0
        cpu.pc = paddr
        sp_addr = 0xfdf0
        cpu.sp = sp_addr
        HHLL = 0x20
        # Cx 0x0, 0xaabb
        cpu.flags = 0b00000100
        chip16_write_phys_memory_BE(cpu, paddr, 0x17002000, 4)
        SIM_continue(1)
        stest.expect_equal(simics.SIM_read_phys_memory(cpu, sp_addr, 2), paddr)
        print "Cx: (pc store) success"
        stest.expect_equal(cpu.sp, sp_addr + 2)
        print "Cx: (sp) success"
        stest.expect_equal(cpu.pc, HHLL)
        print "Cx: (call) success"
        print "Call if Equal (success)"

        paddr = 0x8
        cpu.pc = paddr
        sp_addr = 0xfdf2
        cpu.sp = sp_addr
        HHLL = 0x45
        # Cx 0x1, 0x45
        cpu.flags = 0
        chip16_write_phys_memory_BE(cpu, paddr, 0x17014500, 4)
        SIM_continue(1)
        stest.expect_equal(simics.SIM_read_phys_memory(cpu, sp_addr, 2), paddr)
        stest.expect_equal(cpu.sp, sp_addr + 2)
        stest.expect_equal(cpu.pc, HHLL)
        print "Call if Not equal (success)"

        paddr = 0x10
        cpu.pc = paddr
        sp_addr = 0xfdf4
        cpu.sp = sp_addr
        HHLL = 0xff00
        # Cx 0xb, 0xff00
        cpu.flags = 0b11000000
        chip16_write_phys_memory_BE(cpu, paddr, 0x170b00ff, 4)
        SIM_continue(1)
        stest.expect_equal(simics.SIM_read_phys_memory(cpu, sp_addr, 2), paddr)
        stest.expect_equal(cpu.sp, sp_addr + 2)
        stest.expect_equal(cpu.pc, HHLL)
        print "Call if Signed greater than (success)"

        paddr = 0x10
        cpu.pc = paddr
        sp_addr = 0xfdf6
        cpu.sp = sp_addr
        HHLL = 0xff00
        # Cx 0x3, 0xff00
        cpu.flags = 0b10000000
        chip16_write_phys_memory_BE(cpu, paddr, 0x170300ff, 4)
        SIM_continue(1)
        stest.expect_equal(cpu.sp, sp_addr)
        stest.expect_equal(cpu.pc, paddr + 4)
        print "Not Call (success)"

test_one_availability(conf.chip0)
