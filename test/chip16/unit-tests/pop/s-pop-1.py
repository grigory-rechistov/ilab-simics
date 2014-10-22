# This test checks POP instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):
        paddr = 0x0
        cpu.pc = paddr

        data = 0xDEAD;

        cpu_sp_prev = cpu.sp
        simics.SIM_write_phys_memory(cpu, cpu.sp, data, 2)
        cpu.sp += 2

        # POP RX
        simics.SIM_write_phys_memory(cpu, paddr, 0xC1010000, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.pc, paddr + 4)
        print "XOR: (pc) success"

        stest.expect_equal(cpu.gprs[1], data)
        print "POP: (result) success"

        stest.expect_equal(cpu.sp, cpu_sp_prev)
        print "POP: (sp) success"


test_one_availability(conf.chip0)
