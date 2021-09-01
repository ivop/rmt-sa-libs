STRIP = -s
CFLAGS = -O3 -shared ${STRIP} -DSTEREO -DNO_VOL_ONLY -DNO_CONSOL_SOUND

all: sa_pokey.dll

sa_pokey.dll: pokey.c pokeysnd.c sa_pokey.c pokey.h pokeysnd.h sa_pokey.h
	i686-w64-mingw32-gcc ${CFLAGS} -o sa_pokey.dll pokey.c pokeysnd.c sa_pokey.c -Wl,--subsystem,windows

clean:
	rm -f *~ *.dll

