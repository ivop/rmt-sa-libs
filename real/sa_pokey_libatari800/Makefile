STRIP = -s

all: sa_pokey.dll mirror.xex Makefile

sa_pokey.dll: sa_pokey.c libatari800-win32.a Makefile
	i686-w64-mingw32-gcc -o sa_pokey.dll -shared sa_pokey.c ${STRIP} -Wl,--subsystem,windows libatari800-win32.a -lws2_32

mirror.xex: mirror.s Makefile
	mads -o:mirror.xex mirror.s

clean:
	rm -f *~ sa_pokey.dll mirror.xex
