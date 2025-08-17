INSTDIR	= ~/cc99
CFLAGS	= ${DEBUG}
CC = cc

all: c2

install: c2
	cp c2 ${INSTDIR}/lib

clean:
	rm -f *~ *.o c2 c2.exe

c2: c20.c c21.c c2.h
	$(CC) ${CFLAGS} -o c2 c20.c c21.c

native: c20.c c21.c c2.h
	${INSTDIR}/bin/cc99 ${CFLAGS} -a 0x1000 -o c2.exe c20.c c21.c
