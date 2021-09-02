STRIP = -s
CFLAGS = -O3 -shared ${STRIP}

all: sa_pokey.dll

sa_pokey.dll: pokey.c pokeysnd.c sa_pokey.c pokey.h pokeysnd.h
	i686-w64-mingw32-gcc ${CFLAGS} -o sa_pokey.dll pokey.c pokeysnd.c sa_pokey.c -Wl,--subsystem,windows

clean:
	rm -f *~ *.dll

