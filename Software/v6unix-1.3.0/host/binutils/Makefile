INSTDIR	= ~/cc99
CFLAGS = -O ${DEBUG}
CC = cc

EXES = ld99 nm99 ar99 strip99 size99 out2lmc out2cas
NEXES = ld.exe nm.exe ar.exe strip.exe size.exe

all: ${EXES}

native: ${NEXES}

install: 
	cp ld99 ${INSTDIR}/bin/ld99
	cp nm99 ${INSTDIR}/bin/nm99
	cp ar99 ${INSTDIR}/bin/ar99
	cp strip99 ${INSTDIR}/bin/strip99
	cp size99 ${INSTDIR}/bin/size99
	cp out2lmc ${INSTDIR}/bin/out2lmc
	cp out2cas ${INSTDIR}/bin/out2cas

clean:
	rm -f ${EXES} ${NEXES}
	rm -rf *.dSYM

% : %.c
	${CC} ${CFLAGS} -o $@ $<

ld99: ld.c
	${CC} ${CFLAGS} -o $@ $<
nm99: nm.c
	${CC} ${CFLAGS} -o $@ $<
ar99: ar.c
	${CC} ${CFLAGS} -o $@ $<
strip99: strip.c
	${CC} ${CFLAGS} -o $@ $<
size99: size.c
	${CC} ${CFLAGS} -o $@ $<

ld.exe: ld.c
	${INSTDIR}/bin/cc99 -O -a 0x1000 -o $@ $<

nm.exe: nm.c
	${INSTDIR}/bin/cc99 -O -a 0x1000 -o $@ $<

ar.exe: ar.c
	${INSTDIR}/bin/cc99 -O -a 0x1000 -o $@ $<

strip.exe: strip.c
	${INSTDIR}/bin/cc99 -O -a 0x1000 -o $@ $<

size.exe: size.c
	${INSTDIR}/bin/cc99 -O -a 0x1000 -o $@ $<

out2lmc: out2lmc.c
out2cas: out2cas.c
