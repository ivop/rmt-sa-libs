STRIP = -s

all: sa_pokey.dll apokeysnd.dll sa_c6502.dll

sa_pokey.dll: sa_pokey.c
	i686-w64-mingw32-gcc -o sa_pokey.dll -shared sa_pokey.c ${STRIP} -Wl,--subsystem,windows

apokeysnd.dll: apokeysnd.c
	i686-w64-mingw32-gcc -o apokeysnd.dll -shared apokeysnd.c ${STRIP} -Wl,--subsystem,windows

sa_c6502.dll: sa_c6502.c
	i686-w64-mingw32-gcc -o sa_c6502.dll -shared sa_c6502.c ${STRIP} -Wl,--subsystem,windows

clean:
	rm -f *~ *.dll

