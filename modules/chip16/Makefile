#                                                              -*- Makefile -*-
# Simics module makefile
#

MODULE_CLASSES = chip16

SRC_FILES += chip16.c            \
             chip16-queue.c      \
             chip16-frequency.c  \
             chip16-step.c       \
             chip16-cycle.c      \
             chip16-memory.c     \
             chip16-exec.c       \
             event-queue.c

SRC_FILES += chip16_decoder.c

EXTRA_MODULE_VPATH = cycle-common:genint

SRC_FILES += generic-cycle-iface.c

SIMICS_API := 4.6
THREAD_SAFE:= yes

include $(MODULE_MAKEFILE)
-include $(firstword $(filter %cycle-common,$(VPATH)))/generic-cycle-iface.mk
generic-cycle-iface-test: TESTLIBS+=-lvtutils

GENINT = $(SRC_BASE)/genint/genint.py
chip16_decoder.c: chip16.gi prefix.pre suffix.pre $(GENINT)
	echo "Genint $(<F) -> $@"
	$(PYTHON) $(GENINT) -o $@ $< -p $(<D)/prefix.pre -s $(<D)/suffix.pre
