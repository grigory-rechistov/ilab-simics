# This test checks RET instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):

        paddr = 0x4
        cpu.pc = paddr
        cpu.gprs[1] = 0x1000
        cpu.sp = 0xfdf4
        target = 0xff00
        simics.SIM_write_phys_memory(cpu, cpu.sp - 2, target, 2)

        # RET
        simics.SIM_write_phys_memory(cpu, paddr, 0x15000000, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.sp, 0xfdf2)
        print "RET: (sp) success"

        stest.expect_equal(cpu.pc, target)
        print "RET: (pc) success"

test_one_availability(conf.chip0)
