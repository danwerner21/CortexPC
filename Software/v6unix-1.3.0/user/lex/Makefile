INSTDIR=~/cc99
YACC = ${INSTDIR}/bin/yacc
TCC = ${INSTDIR}/bin/cc99
CC = cc
AR = ${INSTDIR}/bin/ar99

CFLAGS = -O -DUNIX -DLINUX -DASCII ${DEBUG}

all: lex99 

native: lex libln.a

install: lex99 libln.a
	cp lex99 ${INSTDIR}/bin/
	cp libln.a ${INSTDIR}/lib/

lex99: lmain.c y.tab.c sub1.c sub2.c header.c ldefs.c once.c
	${CC} ${CFLAGS} -o lex99 lmain.c y.tab.c sub1.c sub2.c header.c

lex: lmain.c y.tab.c sub1.c sub2.c header.c ldefs.c once.c
	${TCC} ${CFLAGS} -DSMALL -a 0x1000 -o lex lmain.c y.tab.c sub1.c sub2.c header.c

y.tab.c: parser.y
	${YACC} parser.y

libln.a:
	${INSTDIR}/bin/cc99 ${CFLAGS} -c lib/allprint.c lib/main.c lib/reject.c
	${INSTDIR}/bin/cc99 ${CFLAGS} -c lib/yyless.c lib/yywrap.c
	rm -f libln.a
	${AR} r libln.a allprint.o main.o reject.o yyless.o yywrap.o
	rm allprint.o main.o reject.o yyless.o yywrap.o

clean:
	rm -f *.o lex lex99 libln.a y.tab.c

