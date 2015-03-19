# This test checks POP instruction

import stest

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

def test_one_availability(cpu):
        paddr = 0x0
        cpu.pc = paddr

        data = 0xDEAD;

        cpu_sp_prev = cpu.sp
        chip16_write_phys_memory_BE(cpu, cpu.sp, data, 2)
        cpu.sp += 2

        # POP RX
        chip16_write_phys_memory_BE(cpu, paddr, 0xC1010000, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.pc, paddr + 4)
        print "XOR: (pc) success"

        stest.expect_equal(cpu.gprs[1], data)
        print "POP: (result) success"

        stest.expect_equal(cpu.sp, cpu_sp_prev)
        print "POP: (sp) success"


test_one_availability(conf.chip0)
