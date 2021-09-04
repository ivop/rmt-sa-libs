STRIP = -s
CFLAGS = -O3 -shared ${STRIP} -DINJECT_TRACKER_OBX -DCHECK_INIT_MEMORY \
		 -DCHECK_PREVIOUS_MEMORY

all: sa_c6502.dll

sa_c6502.dll: sa_c6502.c cpu.c cpu.h sa_c6502.h Makefile
	i686-w64-mingw32-gcc ${CFLAGS} -o sa_c6502.dll cpu.c sa_c6502.c -Wl,--subsystem,windows

clean:
	rm -f *~ *.dll

