INSTDIR	= ~/cc99
TCC	= ${INSTDIR}/bin/cc99

CFLAGS=-O -c -DNROFF
NFILES=n1.o n2.o n3.o n4.o n5.o n6.o n7.o n8.o n9.o n10.o ni.o nii.o \
	ntab.o hytab.o suftab.o

.c.o:
	${TCC} ${CFLAGS} ${DEBUG} -o $@ $<

nroff:	$(NFILES)
	${TCC} -o nroff -a 0x1000 $(NFILES)

n1.o:	tdef.h d.h v.h tw.h s.h
n2.o:	tdef.h d.h v.h tw.h s.h
n3.o:	tdef.h d.h v.h tw.h s.h
n4.o:	tdef.h d.h v.h tw.h s.h
n5.o:	tdef.h d.h v.h tw.h s.h
n6.o:	tdef.h d.h v.h tw.h s.h
t6.o:	tdef.h d.h v.h tw.h s.h
n7.o:	tdef.h d.h v.h tw.h s.h
n8.o:	tdef.h d.h v.h tw.h s.h
n9.o:	tdef.h d.h v.h tw.h s.h
n10.o:	tdef.h d.h v.h tw.h s.h
t10.o:	tdef.h d.h v.h tw.h s.h
ni.o:	tdef.h d.h v.h tw.h s.h
nii.o:	tdef.h d.h v.h tw.h s.h

hytab.o:	hytab.c
	${TCC} -c hytab.c

suftab.o:	suftab.c
	${TCC} -c suftab.c

clean:
	rm -f nroff *.o
