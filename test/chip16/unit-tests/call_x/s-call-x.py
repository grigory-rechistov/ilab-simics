# This test checks CALL_RX instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):

        paddr = 0x8
        cpu.pc = paddr
        cpu.sp = 0xfdf4
        cpu.gprs[2] = 0x11

        # CALL R2
        chip16_write_phys_memory_BE(cpu, paddr, 0x18020000, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.sp, 0xfdf4 + 2)
        print "CALL_RX: (sp) success"
        stest.expect_equal(simics.SIM_read_phys_memory(cpu, 0xfdf4, 2), paddr)
        print "CALL_RX: (pc store) success"
        stest.expect_equal(cpu.pc, cpu.gprs[2])
        print "CALL_RX: (pc) success"

test_one_availability(conf.chip0)
