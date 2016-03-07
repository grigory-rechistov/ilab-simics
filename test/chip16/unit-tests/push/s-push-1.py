# This test checks PUSH instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        paddr = 0x0
        cpu.pc = paddr

        data = 0xDEAD;
        cpu.gprs[1] = data

        cpu_sp_next = cpu.sp + 2

        # PUSH RX
        chip16_write_phys_memory_BE(cpu, paddr, 0xC0010000, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.pc, paddr + 4)
        print "PUSH: (pc) success"

        data = SIM_read_phys_memory(cpu, cpu.sp - 2, 2)
        stest.expect_equal(cpu.gprs[1], data)
        print "PUSH: (result) success"

        stest.expect_equal(cpu.sp, cpu_sp_next)
        print "PUSH: (sp) success"


test_one_availability(conf.chip0)
