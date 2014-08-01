#                                                              -*- Makefile -*-
# Simics module makefile
#

MODULE_CLASSES = sample-device-c

SRC_FILES= sample-device.c

CFLAGS+= -Wall -Werror

THREAD_SAFE = yes
SIMICS_API := 4.6

# tell C compiler where to find files
EXTRA_MODULE_VPATH := sample-interface

include $(MODULE_MAKEFILE)
