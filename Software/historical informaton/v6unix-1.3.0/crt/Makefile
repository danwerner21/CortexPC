#
# Make run time library.
#
INSTDIR	= ~/cc99
MODEL	= TI990
CONSOLE = TTYEIA
#MODEL	= TI_CTX
#CONSOLE = TTY9902

AS = ${INSTDIR}/bin/as99
AR = ${INSTDIR}/bin/ar99

OBJS = csv.o lsra.o lsrl.o lsla.o lmul.o almul.o idiv.o \
      ldiv.o lrem.o uldiv.o ulrem.o aldiv.o alrem.o auldiv.o \
      aulrem.o

all: crt crt0

install: all
	${AR} r ${INSTDIR}/lib/libcrt.a ${OBJS}
	mv crt0.o ${INSTDIR}/lib/crt0.o
	mv mcrt0.o ${INSTDIR}/lib/mcrt0.o
	mv crtx0.o ${INSTDIR}/lib/crtx0.o

clean:
	rm -f *.o a.out


crt: ${OBJS}

crt0: crt0.o crtx0.o mcrt0.o
		
.s.o:
	${AS} -u -o $@ ../v6/${MODEL}-${CONSOLE}.cfg ../v6/hdr.s $<

csv.o: csv.s
lsra.o: lsra.s
lsrl.o: lsrl.s
lsla.o: lsla.s
lmul.o: lmul.s
almul.o: almul.s
idiv.o: idiv.s
ldiv.o: ldiv.s
lrem.o: lrem.s
uldiv.o: uldiv.s
ulrem.o: ulrem.s
aldiv.o: aldiv.s
alrem.o: alrem.s
auldiv.o: auldiv.s
aulrem.o: aulrem.s
crt0.o:	crt0.s
crtx0.o: crtx0.s

