INSTDIR	= ~/cc99
MODEL	= TI990
CONSOLE = TTYEIA
#MODEL	= TI_CTX
#CONSOLE = TTY9902

TCC	= ${INSTDIR}/bin/cc99
AS	= ${INSTDIR}/bin/as99
LD	= ${INSTDIR}/bin/ld99
NM	= ${INSTDIR}/bin/nm99
STRIP	= ${INSTDIR}/bin/strip99
SIZE	= ${INSTDIR}/bin/size99
UFS	= ${INSTDIR}/bin/ufs


OBJSCTX = low.o mch.o mmu.o drv_cf.o dsk.o
OBJS990 = low.o mch.o mmu_990.o drv_990.o dsk_990.o drv_mt.o drv_prt.o \
	drv_vdt911.o v911io.o drv_tty403.o

UV6OBJS = alloc.o bio.o clock.o conf.o tty.o fio.o iget.o \
	mem.o nami.o pipe.o prf.o rdwri.o sig.o slp.o subr.o sys1.o sys2.o \
	sys3.o sys4.o sysnet.o sysent.o trap.o main.o malloc.o

.c.o:
	${TCC} -O -c -DKERNEL -D${MODEL} -D${CONSOLE} ${DEBUG} -o $@ $<
.s.o:
	${AS} -u -o $@ ${MODEL}-${CONSOLE}.cfg hdr.s $<

all: unix-${MODEL}-${CONSOLE} strip

unix-TI_CTX-TTY9902: ${OBJSCTX} drv_tty9902.o ${UV6OBJS}
	${LD} -x -o unix ${OBJSCTX} drv_tty9902.o ${UV6OBJS}

unix-TI990-TTYEIA: ${OBJS990} drv_ttyeia.o ${UV6OBJS}
	${LD} -x -o unix ${OBJS990} drv_ttyeia.o ${UV6OBJS}

unix-TI990_10A-TTY9902: ${OBJS990} drv_tty9902.o ${UV6OBJS}
	${LD} -x -o unix ${OBJS990} drv_tty9902.o ${UV6OBJS}
unix-TI990_10A-TTYEIA: ${OBJS990} drv_ttyeia.o ${UV6OBJS}
	${LD} -x -o unix ${OBJS990} drv_ttyeia.o ${UV6OBJS}

unix-TI990_12-TTYEIA: ${OBJS990} drv_ttyeia.o ${UV6OBJS}
	${LD} -x -o unix ${OBJS990} drv_ttyeia.o ${UV6OBJS}

strip: unix
	${NM} -ng unix >unix.map
	ls -l unix
	${SIZE} unix

clean:
	rm -f unix ${OBJSCTX} ${OBJS990} ${UV6OBJS} drv_tty*.o *.lst *.map

dsk:
	cp ../user/dsk/filesys.dsk .
	cc99 -o etc/init  -O -a 0x1000 bin/init.c
	cc99 -o bin/login -O -a 0x1000 bin/login.c
	cc99 -o bin/mail  -O -a 0x1000 bin/mail.c
	cc99 -o bin/who   -O -a 0x1000 bin/who.c
	cc99 -o bin/chk   -O -a 0x1000 bin/chk.c
	cc99 -o bin/passwd -O -a 0x1000 bin/passwd.c
	cc99 -o bin/ps    -O -a 0x1000 bin/ps.c
	${UFS} --add filesys.dsk etc/init
	${UFS} --add filesys.dsk etc/passwd
	${UFS} --add filesys.dsk etc/fixperm
	${UFS} --add filesys.dsk etc/utmp
	${UFS} --add filesys.dsk bin/login
	${UFS} --add filesys.dsk bin/mail
	${UFS} --add filesys.dsk bin/who
	${UFS} --add filesys.dsk bin/chk
	${UFS} --add filesys.dsk bin/passwd
	${UFS} --add filesys.dsk bin/ps
	${UFS} --add filesys.dsk home/
	${UFS} --add filesys.dsk home/root/
	${UFS} --add filesys.dsk home/pnr/
	${UFS} --add filesys.dsk unix
	as99 boot.s
	ld99 -o btblk -a 0x8000 a.out
	${UFS} -F -b btblk filesys.dsk
	rm a.out btblk

manual:
	cp ../nroff/term/tab37 usr/lib/term/.
	cp ../nroff/nroff bin/.
	cc99 -o bin/man   -O -a 0x1000 bin/man.c
	cc99 -o bin/col   -O -a 0x1000 bin/col.c
	${UFS} --add filesys.dsk bin/man
	${UFS} --add filesys.dsk bin/col
	${UFS} --add filesys.dsk bin/nroff
	${UFS} --add filesys.dsk man/
	${UFS} --add filesys.dsk man/man0/
	${UFS} --add filesys.dsk man/man0/*
	${UFS} --add filesys.dsk man/man1/
	${UFS} --add filesys.dsk man/man1/*
	${UFS} --add filesys.dsk man/man2/
	${UFS} --add filesys.dsk man/man2/*
	${UFS} --add filesys.dsk man/man3/
	${UFS} --add filesys.dsk man/man3/*
	${UFS} --add filesys.dsk man/man4/
	${UFS} --add filesys.dsk man/man4/*
	${UFS} --add filesys.dsk man/man5/
	${UFS} --add filesys.dsk man/man5/*
	${UFS} --add filesys.dsk man/man6/
	${UFS} --add filesys.dsk man/man6/*
	${UFS} --add filesys.dsk man/man7/
	${UFS} --add filesys.dsk man/man7/*
	${UFS} --add filesys.dsk man/man8/
	${UFS} --add filesys.dsk man/man8/*
	${UFS} --add filesys.dsk usr/lib/
	${UFS} --add filesys.dsk usr/lib/term/
	${UFS} --add filesys.dsk usr/lib/term/*
	${UFS} --add filesys.dsk usr/lib/tmac/
	${UFS} --add filesys.dsk usr/lib/tmac/*

source:
	${UFS} --add filesys.dsk src/
	${UFS} --add filesys.dsk src/cc/
	${UFS} --add filesys.dsk src/cc/*
	${UFS} --add filesys.dsk src/sys/
	${UFS} --add filesys.dsk src/sys/*

alloc.o: alloc.c param.h systm.h filsys.h conf.h buf.h inode.h user.h
bio.o: bio.c param.h user.h buf.h conf.h systm.h proc.h
conf.o: conf.c conf.h
drv_tty403.o: drv_tty403.c param.h conf.h tty.h buf.h user.h
drv_vdt911.o: drv_vdt911.c param.h conf.h tty.h buf.h user.h
drv_tty9902.o: drv_tty9902.c param.h conf.h tty.h buf.h user.h
drv_eia.o: drv_eia.c param.h conf.h tty.h buf.h user.h
dsk.o: dsk.c param.h buf.h user.h
dsk_990.o: dsk_990.c param.h buf.h user.h
drv_mt.o: drv_mt.c param.h conf.h buf.h user.h
drv_prt.o: drv_prt.c param.h conf.h tty.h buf.h user.h
fio.o: fio.c param.h user.h filsys.h file.h conf.h inode.h reg.h
iget.o: iget.c param.h systm.h user.h inode.h filsys.h buf.h
malloc.o: malloc.c param.h proc.h systm.h
mem.o: mem.c param.h user.h conf.h mmu.h
nami.o: nami.c param.h inode.h user.h systm.h buf.h
pipe.o: pipe.c param.h systm.h user.h inode.h file.h reg.h
prf.o: prf.c param.h buf.h conf.h
rdwri.o: rdwri.c param.h inode.h user.h buf.h conf.h systm.h
sig.o: sig.c param.h systm.h user.h proc.h inode.h reg.h
slp.o: slp.c param.h user.h proc.h systm.h file.h inode.h buf.h
subr.o: subr.c param.h conf.h inode.h user.h buf.h systm.h
sys1.o: sys1.c param.h systm.h user.h proc.h buf.h reg.h inode.h file.h
sys2.o: sys2.c param.h systm.h user.h reg.h file.h inode.h
sys3.o: sys3.c param.h systm.h reg.h buf.h filsys.h user.h inode.h file.h conf.h
sys4.o: sys4.c param.h user.h reg.h inode.h systm.h proc.h file.h
sysnet.o: sysnet.c param.h user.h reg.h inode.h systm.h proc.h file.h sysnet.h
sysent.o: sysent.c param.h systm.h
trap.o: trap.c param.h systm.h user.h reg.h
tty.o: tty.c param.h user.h tty.h

low.o: low.s
mch.o: mch.s

drv_cf.o: drv_cf.s
mmu.o: mmu.s

drv_990.o: drv_990.s
mmu_990.o: mmu_990.s
v911io.o: v911io.s
