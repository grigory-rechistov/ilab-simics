# This Software is part of Wind River Simics. The rights to copy, distribute,
# modify, or otherwise make use of this Software may be licensed only
# pursuant to the terms of an applicable Wind River license agreement.
# 
# Copyright 2012-2014 Intel Corporation

# Sample tests for sample-device-c

import stest

# Create sample the device
c_dev = SIM_create_object('sample-device-c', 'sample_dev_c', [])

def test_device():
    # Check that value is read correct.
    for val in (0, 10, 73):
        c_dev.value = val
        stest.expect_equal(c_dev.value, val)

    # Check that simple_method outputs log.
    simple_method = c_dev.iface.sample.simple_method
    stest.expect_log(simple_method, (1,), log_type = "info")

    def add_log(logstr):
        c_dev.add_log = logstr

    logstr = "testing"
    # Check that add_log attribute outputs a log.
    stest.expect_log(add_log, (logstr,), log_type = "info", msg = logstr)

test_device()

print "All tests passed."
