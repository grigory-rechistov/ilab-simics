# This test checks CALL HHLL instruction

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

def test_one_availability(cpu):

        paddr = 0x8
        cpu.pc = paddr
        cpu.sp = 0xfdf4
        HHLL = 0xff00

        # CALL HHLL
        simics.SIM_write_phys_memory(cpu, paddr, 0x140000ff, 4)
        SIM_continue(1)

        stest.expect_equal(cpu.sp, 0xfdf4 + 2)
        print "CALL_HHLL: (sp) success"

        # I don't know how to perform a write memory test
        # other then just manually cheking it from simics
        # nevertheless, write memory does work:

        # simics> phys_mem0.get 0xfdf4 2
        # 8 (LE)

        stest.expect_equal(cpu.pc, HHLL)
        print "CALL_HHLL: (pc) success"

test_one_availability(conf.chip0)
