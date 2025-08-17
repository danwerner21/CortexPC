CC	= cc
CFLAGS	= -O ${DEBUG}
LDFLAGS	= ${DEBUG}
INSTDIR	= /usr/local

UFSOBJS	= fsutil.o superblock.o block.c inode.o create.o check.o file.o
MKDOBJS = mkudisk.o support.o binloader.o

PROGS	= ufs mkudisk

# For Mac OS X
#LIBS	= -largp

all: $(PROGS) uboot.obj

install: $(PROGS)
	cp $(PROGS) ${INSTDIR}/bin/

clean:
	rm -f *~ *.o *.lst *.dis $(PROGS) uboot.obj

ufs: $(UFSOBJS)
	$(CC) $(LDFLAGS) -o $@ $(UFSOBJS) $(LIBS)

mkudisk: $(MKDOBJS)
	$(CC) $(LDFLAGS) -o $@ $(MKDOBJS) $(LIBS)

uboot.obj: uboot.asm
	asm990 -o uboot.obj uboot.asm

mkudisk.o: mkudisk.c support.h simdsk.h
