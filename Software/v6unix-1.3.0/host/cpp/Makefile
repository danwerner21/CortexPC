INSTDIR	= ~/cc99
CFLAGS	= -g -DFLEXNAMES ${CC_CFLAGS} ${DEBUG}
YACC = ${INSTDIR}/bin/yacc
TCC = ${INSTDIR}/bin/cc99
CC = cc

all: cpp

install: cpp
	cp cpp ${INSTDIR}/bin/cpp99

clean:
	rm -f *~ *.s *.o cpp

cpp: cpp.c cpy.c yylex.c
	$(CC) $(CFLAGS) -o cpp cpp.c cpy.c

cpy.c: cpy.y
	${YACC} cpy.y
	mv y.tab.c cpy.c

