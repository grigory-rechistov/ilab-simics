# Test verifies correctness of checksum calculation and checking

import stest

cli.run_command("run-python-file %s/targets/chip16/machine.py" % conf.sim.workspace)

# this file consists delibirately damaged raw data, while checksum left untouched
incorrect_crc = "%s/test/chip16/unit-tests/binary/file2.c16" % conf.sim.workspace
cpu = current_processor()

got_exception = False

try:
    cli.run_command("chip16-load-binary %s" % incorrect_crc)
except CliError:
    got_exception = True

stest.expect_true(got_exception)
stest.expect_equal(simics.SIM_read_phys_memory(cpu, 0x0, 8), 0)
print "Load-binary: (incorrect_crc) success"
