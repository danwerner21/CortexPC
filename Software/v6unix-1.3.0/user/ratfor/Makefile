INSTDIR=~/cc99
TCC=${INSTDIR}/bin/cc99
YACC=${INSTDIR}/bin/yacc

CFLAGS = -O -c

.c.o:
	${TCC} ${CFLAGS} ${DEBUG} -o $@ $<

all:	ratfor

ratfor:	r0.o r1.o r2.o rio.o rlook.o rlex.o y.tab.o
	${TCC} -a 0x1000 -o ratfor r*.o y.tab.o

clean:
	rm -f ratfor *.o

r0.o:	r.h y.tab.h r0.c
r1.o:	r.h y.tab.h r1.c
r2.o:	r.h y.tab.h r2.c
rio.o:	r.h y.tab.h rio.c
rlook.o:	r.h y.tab.h rlook.c
rlex.o:	r.h y.tab.h rlex.c
y.tab.c:	r.g
	${YACC} -d r.g
y.tab.h:	r.g
	${YACC} -d r.g
