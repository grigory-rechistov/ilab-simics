#                                                              -*- Makefile -*-
# Simics module makefile
#

MODULE_CLASSES = snd16

SRC_FILES = snd16.c audio.c
#PYTHON_FILES = module_load.py

THREAD_SAFE = yes
SIMICS_API := 4.6

# tell C compiler where to find files
EXTRA_MODULE_VPATH := sample-interface

EXTRA_MODULE_VPATH += sdl

LDFLAGS+= -L$(SIMICS_WORKSPACE)/modules/sdl/lib
LIBS += -Wl,-rpath,$(SIMICS_WORKSPACE)/modules/sdl/lib -lSDL2 -lpthread -Wl,--no-undefined -lm -lasound -lm -ldl -lpthread -lpulse-simple -lpulse -lX11 -lXext -lpthread -lrt

include $(MODULE_MAKEFILE)
