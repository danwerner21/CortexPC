INSTDIR=~/cc99
CFLAGS=-O -DUNIX -DLINUX
CC=cc

all: yacc install

install:
	cp yacc ${INSTDIR}/bin/yacc
	cp yaccpar ${INSTDIR}/lib/yaccpar

yacc: y1.c y2.c y3.c y4.c dextern files
	${CC} ${CFLAGS} ${CC_CFLAGS} -o yacc y?.c

clean:
	rm -f *.o yacc yacc.exe



