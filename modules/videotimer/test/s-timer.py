# This Software is part of Wind River Simics. The rights to copy, distribute,
# modify, or otherwise make use of this Software may be licensed only
# pursuant to the terms of an applicable Wind River license agreement.
# 
# Copyright 2012-2014 Intel Corporation

import dev_util
from stest import expect_equal

irq_dev = dev_util.Dev([dev_util.Signal])

clock = pre_conf_object('clock', 'clock')
clock.freq_mhz = 333

timer = pre_conf_object('timer', 'videotimer')
timer.queue = clock
timer.irq_dev = irq_dev.obj

SIM_add_configuration([clock, timer], None)
timer = conf.timer

ref = 250   # reference counter value
cnt = 10    # counter start value

counter   = dev_util.Register_BE((timer, "regs", 0x00), size = 2)
reference = dev_util.Register_BE((timer, "regs", 0x02), size = 2)
step      = dev_util.Register_BE((timer, "regs", 0x04), size = 2)
config    = dev_util.Register_BE((timer, "regs", 0x06), size = 2,
                                 bitfield = dev_util.Bitfield_BE(
                                     {'clear_on_match'  : 14,
                                      'interrupt_enable': 15},
                                     bits = 16))

def test_timer(stp = 1, ien = 0, clr = 0):
    step.write(stp)
    reference.write(ref)
    config.write(clear_on_match = clr, interrupt_enable = ien)
    counter.write(cnt)
    irq_dev.signal.spikes = 0

    # if step is 0, the timer stops
    if stp == 0:
        expect_equal(counter.read(), cnt)
        expect_equal(irq_dev.signal.spikes, 0)
        return

    # let time fly to one cycle before reference reached
    SIM_continue((ref - cnt) * stp - 1)

    # check the counter value just before reference reached
    expect_equal(counter.read(), (ref - 1))
    # also check the output interrupt
    expect_equal(irq_dev.signal.spikes, 0)

    SIM_continue(1)

    # check the counter value when reference reached
    # if clear_on_match is set, the counter should be cleared
    if clr == 1:
        expect_equal(counter.read(), 0)
    else:
        expect_equal(counter.read(), ref)

    # if interrupt_enable is set, check whether interrupt is raised 
    if ien == 0:
        expect_equal(irq_dev.signal.spikes, 0)
    else:
        expect_equal(irq_dev.signal.spikes, 1)

test_timer(stp = 0, ien = 0, clr = 0)
test_timer(stp = 1, ien = 0, clr = 0)
test_timer(stp = 1, ien = 1, clr = 0)
test_timer(stp = 2, ien = 1, clr = 1)

print "s-timer: all tests passed!"

