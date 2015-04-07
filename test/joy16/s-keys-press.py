# This test checks working condition of joystick

import stest
import dev_util

cli.run_command("run-python-file %s/test/chip16-setup.py" % conf.sim.workspace)

cpu = conf.chip0
joy = conf.joy0

# Instructions how to use the simple_method for joystick
#
#        SDL_SCANCODE_RIGHT = 79 ------
#        SDL_SCANCODE_LEFT = 80 -------  (*SCANCODE* | (3<<29)) for keydown
#        SDL_SCANCODE_DOWN = 81 -------  (*SCANCODE* | (1<<30)) for keyup
#        SDL_SCANCODE_UP = 82 ---------
#
# SELECT SDLK_TAB = 9 -----------------
# START  SDLK_RETURN = 13 -------------  (*SDLK* | (1<<29)) for keydown
#   A    SDLK_f = 102 -----------------  (*SDLK*)           for keyup
#   B    SDLK_g = 103 -----------------
#
#   Joystick register mapping. Firt 8 bits used. Last 8 bits reserved.
#
#   8       7       6       5       4       3       2       1       0
#   |-------|-------|-------|-------|-------|-------|-------|-------|
#   |   B   |   A   | START |SELECT | RIGHT | LEFT  | DOWN  |  UP   |

# Passing SDL_KEYDOWN event for UP button
joy.iface.sample.simple_method(82 | (3<<29))
joy.iface.signal.signal_raise()
joy.iface.signal.signal_lower()
stest.expect_equal(simics.SIM_read_phys_memory(cpu, 0xfff0, 2), 1)
print "Joystick: (key_pressed) success"

# Passing SDL_KEYUP event for UP button
joy.iface.sample.simple_method(82 | (1<<30))
joy.iface.signal.signal_raise()
joy.iface.signal.signal_lower()
stest.expect_equal(simics.SIM_read_phys_memory(cpu, 0xfff0, 2), 0)
print "Joystick: (key_unpressed) success"

# Passing SDL_KEYDOWN event for SELECT button
joy.iface.sample.simple_method(9 | (1<<29))
# Passing SDL_KEYDOWN event for LEFT button
joy.iface.sample.simple_method(80 | (3<<29))
joy.iface.signal.signal_raise()
joy.iface.signal.signal_lower()
stest.expect_equal(simics.SIM_read_phys_memory(cpu, 0xfff0, 2), 0b00010100)   
print "Joystick: (keys_pressed) success"
