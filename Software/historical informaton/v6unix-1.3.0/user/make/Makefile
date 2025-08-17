INSTDIR	= ~/cc99
TCC	= ${INSTDIR}/bin/cc99
YACC	= ${INSTDIR}/bin/yacc


# Description file for the Make command

OBJECTS = ident.o main.o doname.o misc.o files.o dosys.o gram.o 
LIBES= 
CFLAGS = -O -c -DUNIX -DLINUX

.c.o:
	${TCC} ${CFLAGS} ${DEBUG} -o $@ $<

all:	make

make:  $(OBJECTS)
	$(TCC) -o make -a 0x1000 $(OBJECTS) $(LIBES)

$(OBJECTS):  defs

clean:
	-rm -f *.o make

install:
	cp make /bin/make

gram.c: gram.y
	$(YACC) gram.y
	mv y.tab.c gram.c
