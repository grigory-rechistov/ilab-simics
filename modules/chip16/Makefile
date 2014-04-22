#                                                              -*- Makefile -*-
# Simics module makefile
#

MODULE_CLASSES = sample-risc sample-risc-core

SRC_FILES += sample-risc.c            \
             sample-risc-queue.c      \
             sample-risc-frequency.c  \
             sample-risc-step.c       \
             sample-risc-cycle.c      \
             sample-risc-memory.c     \
             sample-risc-exec.c       \
             event-queue.c

EXTRA_MODULE_VPATH = cycle-common

SRC_FILES += generic-cycle-iface.c

SIMICS_API := 4.6
THREAD_SAFE:= yes

include $(MODULE_MAKEFILE)
-include $(firstword $(filter %cycle-common,$(VPATH)))/generic-cycle-iface.mk
generic-cycle-iface-test: TESTLIBS+=-lvtutils
