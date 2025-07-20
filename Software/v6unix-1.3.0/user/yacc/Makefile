INSTDIR=~/cc99
CFLAGS=-O
CC=cc
TCC=${INSTDIR}/bin/cc99

all: yacc install

install:
	cp yacc ${INSTDIR}/bin/yacc
	cp yaccpar ${INSTDIR}/lib/yaccpar

yacc: y1.c y2.c y3.c y4.c dextern files
	${CC} ${CFLAGS} ${CC_CFLAGS} -o yacc y?.c

native:
	${TCC} ${CFLAGS} -o yacc.exe -a 0x1000 y?.c

clean:
	rm -f *.o yacc yacc.exe



