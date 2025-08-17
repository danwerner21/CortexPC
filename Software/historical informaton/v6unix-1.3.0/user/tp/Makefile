INSTDIR=~/cc99
TCC = $(INSTDIR)/bin/cc99

CFLAGS=-O -c

.c.o:
	${TCC} ${CFLAGS} ${DEBUG} -o $@ $<

all:	tp

clean:
	rm -f tp *.o

tp:	tp0.o tp1.o tp2.o tp3.o
	$(TCC) -a 0x1000 -o tp tp0.o tp1.o tp2.o tp3.o

tp0.c tp1.c tp2.c tp3.c: tp.h tp_defs.h
