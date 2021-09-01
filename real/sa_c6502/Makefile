STRIP = -s

all: sa_c6502.dll

sa_c6502.dll: sa_c6502.c cpu.c stdafx.c cpu.h stdafx.h sa_c6502.h
	i686-w64-mingw32-gcc -O3 -o sa_c6502.dll -shared cpu.c sa_c6502.c ${STRIP} -Wl,--subsystem,windows

clean:
	rm -f *~ *.dll

