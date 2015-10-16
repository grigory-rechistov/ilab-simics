
# The default platform
simchip16: chip16 joy16 graph16 videotimer snd16

test: NOERROR=--noerror
test: checkstyle

checkstyle:
	./tools/styler.py modules $(NOERROR)
