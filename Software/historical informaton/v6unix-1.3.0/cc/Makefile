INSTDIR	= ~/cc99
CFLAGS	= -O ${CC_CFLAGS}
CC = cc

all: cc99

install: cc99
	cp cc99 ${INSTDIR}/bin/cc99

clean:
	rm -f *~ *.o cc99 cc99.exe

cc99:
	${CC} ${CFLAGS} -o cc99 cc.c

native:
	${INSTDIR}/bin/cc99 -O -a 0x1000 -o cc99.exe cc.c
