#
# Make stdlib part of std C library.
#
INSTDIR	= /usr/local

TCC = ${INSTDIR}/bin/cc99
AR = ${INSTDIR}/bin/ar99
AS = ${INSTDIR}/bin/as99

OBJS = atoi.o atol.o malloc.o mkstemp.o strchr.o strncpy.o \
      calloc.o memccpy.o opendir.o strcmp.o strrchr.o \
      closedir.o memchr.o qsort.o  strcpy.o mktemp.o access.o \
      ctime.o ctype.o memcmp.o readdir.o strlen.o system.o \
      execvp.o memcpy.o readmtab.o strncat.o ttyname.o \
      isatty.o memset.o strcat.o strncmp.o errlst.o perror.o \
      ttyn.o crypt.o setjmp.o reset.o nlist.o mon.o ludiv.o

all: stdlib

install: all
	${AR} r ${INSTDIR}/lib/libc.a ${OBJS}

clean:
	rm -f *.o

stdlib: ${OBJS}

.c.o:
	${TCC} -O -c -o $@ $<

.s.o:
	${AS} -u -o $@ $<

atoi.o: atoi.c
atol.o: atol.c
malloc.o: malloc.c
mkstemp.o: mkstemp.c
strchr.o: strchr.c
strncpy.o: strncpy.c
calloc.o: calloc.c
memccpy.o: memccpy.c
opendir.o: opendir.c
strcmp.o: strcmp.c
strrchr.o: strrchr.c
closedir.o: closedir.c
memchr.o: memchr.c
qsort.o: qsort.c
strcpy.o: strcpy.c
ctime.o: ctime.c
ctype.o: ctype.c
memcmp.o: memcmp.c
mktemp.o: mktemp.c
access.o: access.c
readdir.o: readdir.c
strlen.o: strlen.c
execvp.o: execvp.c
memcpy.o: memcpy.c
readmtab.o: readmtab.c
strncat.o: strncat.c
isatty.o: isatty.c
ttyname.o: ttyname.c
memset.o: memset.c
strcat.o: strcat.c
strncmp.o: strncmp.c
errlst.o: errlst.c
perror.o: perror.c
crypt.o: crypt.s
setjmp.o: setjmp.s
reset.o: reset.s
ttyn.o: ttyn.c
nlist.o: nlist.c
mon.o: mon.c
system.o: system.c
ludiv.o: ludiv.s

