# This test checks Cx instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):

        paddr = 0x8
        cpu.pc = paddr
        cpu.sp = 0xfdf4
        HHLL = 0xaabb

        # Cx HHLL
        chip16_write_phys_memory_BE(cpu, paddr, 0x1701bbaa, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.sp, 0xfdf4 + 2)
        print "Cx: (sp) success"
        stest.expect_equal(simics.SIM_read_phys_memory(cpu, 0xfdf4, 2), paddr)
        print "Cx: (pc store) success"
        stest.expect_equal(cpu.pc, HHLL)
        print "Cx: (pc) success"

        chip16_write_phys_memory_BE(cpu, paddr, 0x1700bbaa, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.pc, paddr)
        print "Cx: (not jump) success"


test_one_availability(conf.chip0)
