INSTDIR	= ~/cc99
CC = cc
CFLAGS = -O ${DEBUG}

OBJS = assem.o lexer.o machine.o output.o tables.o expr.o list.o main.o symbol.o
SRCS = assem.c lexer.c machine.c output.c tables.c expr.c list.c main.c symbol.c

all: as99

install: as99
	cp as99 ${INSTDIR}/bin/as99

clean:
	rm -f *~ *.o as99 as99.exe

as99: ${OBJS}
	${CC} ${CFLAGS} -o as99 ${OBJS}

native:
	${INSTDIR}/bin/cc99 ${CFLAGS} -a 0x1000 -o as99.exe ${SRCS}
	rm *.o

assem.o: assem.c as.h
lexer.o: lexer.c as.h
machine.o: machine.c as.h
output.o: output.c as.h
tables.o: tables.c as.h
expr.o: expr.c as.h
list.o: list.c as.h
main.o: main.c as.h
symbol.o: symbol.c as.h
