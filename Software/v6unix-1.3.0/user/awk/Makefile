INSTDIR=~/cc99
TCC=${INSTDIR}/bin/cc99
YACC=${INSTDIR}/bin/yacc
LEX=${INSTDIR}/bin/lex99
CC = cc

CFLAGS=-O -c
YFLAGS=-d

.c.o:
	${TCC} ${CFLAGS} ${DEBUG} -o $@ $<

all:	awk

FILES=awk.lx.o b.o main.o token.o tran.o lib.o run.o parse.o proctab.o 

awk:	$(FILES) awk.g.o
	${TCC} -o awk -a 0x1000 awk.g.o $(FILES) 

clean:
	rm -f ${FILES} awk awk.g.o
	rm -f proc proc.o

y.tab.h:	awk.g.o

awk.h:	y.tab.h
	-cmp -s y.tab.h awk.h || cp y.tab.h awk.h

$(FILES):	awk.h awk.def

token.c:	awk.h
	ed - <tokenscript
	rm temp

proctab.c:	proc
	./proc > proctab.c

proc:	awk.h proc.c token.c
	${CC} -o proc proc.c token.c

awk.lx.c: awk.lx.l
	${LEX} -t awk.lx.l >awk.lx.c

awk.g.c: awk.g.y
	${YACC} -d awk.g.y
	mv -f y.tab.c awk.g.c
