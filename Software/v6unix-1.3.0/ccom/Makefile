INSTDIR	= ~/cc99
CFLAGS	= ${DEBUG}
CC = cc

all: c0 c1

install: c0 c1
	cp c0 ${INSTDIR}/lib/c0
	cp c1 ${INSTDIR}/lib/c1

clean:
	rm -f *~ *.o c0 c1 cvopt *.exe

c0: c00.c c01.c c02.c c03.c c04.c c05.c c0.h
	${CC} ${CFLAGS} -o c0 c00.c c01.c c02.c c03.c c04.c c05.c

c1: c10.c c11.c c12.c c13.c table.c c0.h
	${CC} $(CFLAGS) -o c1 c10.c c11.c c12.c c13.c table.c

table.c: optable cvopt
	./cvopt < optable > table.c

cvopt: cvopt.c
	${CC} $(CFLAGS) $(LDFLAGS) -o cvopt cvopt.c

native: table.c
	${INSTDIR}/bin/cc99 -O -o c0.exe -a 0x1000 c0?.c
	${INSTDIR}/bin/cc99 -O -o c1.exe -a 0x1000 c1?.c table.c
