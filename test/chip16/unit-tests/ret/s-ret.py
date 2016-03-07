# This test checks RET instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):

        paddr = 0x4
        cpu.pc = paddr
        cpu.gprs[1] = 0x1000
        cpu.sp = 0xfdf4
        target = 0xff00
        SIM_write_phys_memory(cpu, cpu.sp - 2, target, 2)

        # RET
        chip16_write_phys_memory_BE(cpu, paddr, 0x15000000, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.sp, 0xfdf2)
        print "RET: (sp) success"

        stest.expect_equal(cpu.pc, target + 4)
        print "RET: (pc) success"

test_one_availability(conf.chip0)
