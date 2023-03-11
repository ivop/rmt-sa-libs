STRIP = -s
CFLAGS = -O3 -shared ${STRIP}

all: sa_pokey.dll sa_pokey64.dll

sa_pokey.dll: pokey.c pokeysnd.c sa_pokey.c pokey.h pokeysnd.h
	i686-w64-mingw32-gcc ${CFLAGS} -o sa_pokey.dll pokey.c pokeysnd.c sa_pokey.c -Wl,--subsystem,windows
	
sa_pokey64.dll: pokey.c pokeysnd.c sa_pokey.c pokey.h pokeysnd.h
	x86_64-w64-mingw32-gcc ${CFLAGS} -o sa_pokey64.dll pokey.c pokeysnd.c sa_pokey.c -Wl,--subsystem,windows

clean:
	rm -f *~ *.dll

