INSTDIR	= ~/cc99
MODEL	= TI990
#MODEL	= TI_CTX

AS	= ${INSTDIR}/bin/as99
LD	= ${INSTDIR}/bin/ld99
UFS	= ${INSTDIR}/bin/ufs
STRIP	= ${INSTDIR}/bin/strip99
MKUDISK	= ${INSTDIR}/bin/mkudisk

DISK	= DS50
MAXSIZE = 33024000


all: unix net swp boot-${MODEL} dsk-${MODEL}

dsk-TI990 dsk-TI990_12 dsk-TI990_10A: dsk0 mkudisk-dsk0 
dsk-TI_CTX: dsk0

boot-TI990 boot-TI990_12 boot-TI990_10A: ../../v6/boot_990.s
	${AS} -o boot.o ../../v6/boot_990.s
	${LD} -o boot -a 0x8000 boot.o

boot-TI_CTX: ../../v6/boot.s
	${AS} -o boot.o ../../v6/boot.s
	${LD} -o boot -a 0x8000 boot.o

unix: ../../v6/unix
	cp ../../v6/unix unix

net: ../../bbnnet/net
	cp ../../bbnnet/net etc/
	cp ../../bbnnet/tcptest etc/
	cp ../../bbnnet/netstat bin/
	cp ../../bbnnet/libnet.a lib/
	cp ../../bbnnet/gateways etc/
	cp ../../bbnnet/ifaces etc/
	cp ../../bbnnet/thishost etc/
	cp ../../bbnnet/hosts etc/

swp:
	@if [ ! -f swp ]; then \
	   dd if=/dev/zero of=swp bs=512 count=1024 ;\
	fi

dsk0: etc/mtab
	${STRIP} bin/* etc/init etc/glob etc/mount etc/umount
	echo "root:zCunRu2C:0:0::/:/bin/sh" >etc/passwd
	echo "pnr:ugiTjezp:100:0::/home/pnr:/bin/sh" >>etc/passwd
	echo "dgp:czpAd86A:101:0::/home/dgp:/bin/sh" >>etc/passwd
	echo "guest:73Qvsr3r:102:0::/home/guest:/bin/sh" >>etc/passwd
	echo "wheel::0:root,pnr,dgp,guest" >etc/group

	rm -f filesys.dsk
	${UFS} --new --size=${MAXSIZE} filesys.dsk
	${UFS} --add --perm=0744 filesys.dsk unix
	${UFS} --add --perm=0755 filesys.dsk bin/
	${UFS} --add --perm=0755 filesys.dsk lib/
	${UFS} --add --perm=0755 filesys.dsk usr/
	${UFS} --add --perm=0755 filesys.dsk mnt/
	${UFS} --add --perm=0755 filesys.dsk usr/adm/
	${UFS} --add --perm=0755 filesys.dsk usr/adm/wtmp
	${UFS} --add --perm=0755 filesys.dsk usr/include/
	${UFS} --add --perm=0755 filesys.dsk usr/include/sys/
	${UFS} --add --perm=0644 filesys.dsk usr/include/*.h
	${UFS} --add --perm=0644 filesys.dsk usr/include/sys/*.h
	${UFS} --add --perm=0755 filesys.dsk usr/lib/
	${UFS} --add --perm=0755 filesys.dsk usr/share/
	${UFS} --add --perm=0644 filesys.dsk usr/share/*
	${UFS} --add --perm=0644 filesys.dsk usr/lib/yaccpar
	${UFS} --add --perm=0755 filesys.dsk usr/lib/term/
	${UFS} --add --perm=0644 filesys.dsk usr/lib/term/*
	${UFS} --add --perm=0755 filesys.dsk usr/lib/tmac/
	${UFS} --add --perm=0644 filesys.dsk usr/lib/tmac/*
	${UFS} --add --perm=0755 filesys.dsk usr/lib/lex/
	${UFS} --add --perm=0644 filesys.dsk usr/lib/lex/*
	${UFS} --add --perm=0755 filesys.dsk etc/
	${UFS} --add --perm=0644 filesys.dsk etc/motd etc/passwd etc/utmp
	${UFS} --add --perm=0644 filesys.dsk etc/group etc/passwd etc/utmp
	${UFS} --add --perm=0644 filesys.dsk etc/gateways etc/hosts etc/ifaces
	${UFS} --add --perm=0644 filesys.dsk etc/thishost etc/ttys
	${UFS} --add --perm=0755 filesys.dsk etc/*
	${UFS} --add --perm=0755 filesys.dsk home/
	${UFS} --add --perm=0755 filesys.dsk dev/
	${UFS} --add --perm=0777 filesys.dsk tmp/
	${UFS} --add --perm=04755 filesys.dsk bin/login bin/passwd bin/newgrp
	${UFS} --add --perm=04755 filesys.dsk bin/mail bin/mkdir bin/ps
	${UFS} --add --perm=0755 filesys.dsk bin/*
	${UFS} --add --perm=0644 filesys.dsk lib/crt0.o lib/libc.a lib/libcrt.a
	${UFS} --add --perm=0755 filesys.dsk lib/*
	${UFS} --add --perm=0755 filesys.dsk man/
	${UFS} --add --perm=0755 filesys.dsk man/man0/
	${UFS} --add --perm=0644 filesys.dsk man/man0/*
	${UFS} --add --perm=0755 filesys.dsk man/man1/
	${UFS} --add --perm=0644 filesys.dsk man/man1/*
	${UFS} --add --perm=0755 filesys.dsk man/man2/
	${UFS} --add --perm=0644 filesys.dsk man/man2/*
	${UFS} --add --perm=0755 filesys.dsk man/man3/
	${UFS} --add --perm=0644 filesys.dsk man/man3/*
	${UFS} --add --perm=0755 filesys.dsk man/man4/
	${UFS} --add --perm=0644 filesys.dsk man/man4/*
	${UFS} --add --perm=0755 filesys.dsk man/man5/
	${UFS} --add --perm=0644 filesys.dsk man/man5/*
	${UFS} --add --perm=0755 filesys.dsk man/man6/
	${UFS} --add --perm=0644 filesys.dsk man/man6/*
	${UFS} --add --perm=0755 filesys.dsk man/man7/
	${UFS} --add --perm=0644 filesys.dsk man/man7/*
	${UFS} --add --perm=0755 filesys.dsk man/man8/
	${UFS} --add --perm=0644 filesys.dsk man/man8/*
	${UFS} --add --perm=0777 filesys.dsk man/cat0/
	${UFS} --add --perm=0777 filesys.dsk man/cat1/
	${UFS} --add --perm=0777 filesys.dsk man/cat2/
	${UFS} --add --perm=0777 filesys.dsk man/cat3/
	${UFS} --add --perm=0777 filesys.dsk man/cat4/
	${UFS} --add --perm=0777 filesys.dsk man/cat5/
	${UFS} --add --perm=0777 filesys.dsk man/cat6/
	${UFS} --add --perm=0777 filesys.dsk man/cat7/
	${UFS} --add --perm=0777 filesys.dsk man/cat8/
	${UFS} --add --perm=0600 filesys.dsk dev/tty0!c0:0 
	${UFS} --add --perm=0644 filesys.dsk dev/dsk0!b0:0
	${UFS} --add --perm=0644 filesys.dsk dev/dsk1!b0:16
	${UFS} --add --perm=0644 filesys.dsk dev/dsk2!b0:32
	${UFS} --add --perm=0644 filesys.dsk dev/dsk3!b0:48
	${UFS} --add --perm=0600 filesys.dsk dev/mem!c1:0
	${UFS} --add --perm=0600 filesys.dsk dev/kmem!c1:1
	${UFS} --add --perm=0666 filesys.dsk dev/null!c1:2
	${UFS} --add --perm=0666 filesys.dsk dev/zero!c1:3
	${UFS} --add --perm=0666 filesys.dsk dev/net!c3:0
	${UFS} --add --perm=0600 filesys.dsk dev/tty3!c4:0 
	${UFS} --add --perm=0600 filesys.dsk dev/tty4!c4:1 
	${UFS} --add --perm=0600 filesys.dsk dev/tty5!c4:2 
	${UFS} --add --perm=0600 filesys.dsk dev/tty6!c4:3 
	${UFS} --add --perm=0666 filesys.dsk dev/mt0!c5:0
	${UFS} --add --perm=0666 filesys.dsk dev/nmt0!c5:128
	${UFS} --add --perm=0666 filesys.dsk dev/lp0!c6:0
	${UFS} --add --perm=0600 filesys.dsk dev/tty1!c7:0 
	${UFS} --add --perm=0600 filesys.dsk dev/tty2!c7:1 
	${UFS} --add --perm=0755 --uid=100 filesys.dsk home/pnr/
	${UFS} --add --perm=0755 --uid=101 filesys.dsk home/dgp/
	${UFS} --add --perm=0644 --uid=101 filesys.dsk home/dgp/*
	${UFS} --add --perm=0755 --uid=102 filesys.dsk home/guest/
	${UFS} --add --perm=0644 --uid=102 filesys.dsk home/guest/*
	${UFS} -F -b boot filesys.dsk
	cat swp >>filesys.dsk

dsk1:
	rm -f userdsk.dsk
	${UFS} --new --size=${MAXSIZE} userdsk.dsk
	${UFS} --add --perm=0755 --uid=101 userdsk.dsk dgp/

etc/mtab: mkmtab
	./mkmtab etc/mtab dsk0 /
	
mkmtab: mkmtab.c

mkudisk-dsk0:
	${MKUDISK} -v -d ${DISK} -i UNIX -b ../../host/fsutil/uboot.obj \
		v6unix.dsk filesys.dsk

mkudisk-dsk1:
	${MKUDISK} -v -l USERV6 -d ${DISK} v6user.dsk userdsk.dsk

install:
	cp filesys.dsk ../../v6/filesys.dsk

clean:
	rm -f filesys.dsk unix swp boot boot.o v6unix.dsk
	rm -f userdsk.dsk v6user.dsk
	rm -f mkmtab
	rm -rf man/* usr/*
