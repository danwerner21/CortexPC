INSTDIR	= ~/cc99
CFLAGS	= -g -DFLEXNAMES ${CC_CFLAGS} ${DEBUG}
YACC = ${INSTDIR}/bin/yacc
CC = cc

all: cpp

install: cpp
	cp cpp ${INSTDIR}/bin/cpp99

native: cpp.c cpy.c yylex.c
	${INSTDIR}/bin/cc99 -a 0x1000 -o cpp.exe -O cpp.c cpy.c

clean:
	rm -f *~ *.s *.o cpp cpp.exe

cpp: cpp.c cpy.c yylex.c
	$(CC) $(CFLAGS) -o cpp cpp.c cpy.c

cpy.c: cpy.y
	${YACC} cpy.y
	mv y.tab.c cpy.c

