INSTDIR	= ~/cc99
CFLAGS	= -w -O -I../v6 ${CC_CFLAGS}
TCC = ${INSTDIR}/bin/cc99

all: cdb

clean:
	rm -f *~ *.o cdb

cdb:
	${TCC} ${CFLAGS} -o cdb -a 0x1000 cdb1.c cdb2.c

cdb: cdb1.c cdb2.c

