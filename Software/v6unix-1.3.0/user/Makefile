INSTDIR = ~/cc99
MODEL	= TI990
#MODEL	= TI_CTX

TCC	= ${INSTDIR}/bin/cc99
CFLAGS	= -O -a 0x1000 -D${MODEL} ${DEBUG} -I../v6
YACC	= ${INSTDIR}/bin/yacc

EXES = dsk/bin/cal dsk/bin/cat dsk/bin/cp dsk/bin/date dsk/bin/echo dsk/bin/ed \
	dsk/bin/ln dsk/bin/ls dsk/bin/nm dsk/bin/mkdir dsk/bin/col dsk/bin/dd \
	dsk/bin/rmdir dsk/bin/chmod dsk/bin/mv dsk/bin/pwd dsk/bin/idate \
	dsk/bin/rm dsk/bin/reloc dsk/bin/wc dsk/bin/kill dsk/bin/chown \
	dsk/bin/chgrp dsk/bin/sh dsk/bin/size dsk/bin/strip dsk/bin/sync \
	dsk/bin/uname dsk/bin/touch dsk/bin/basename dsk/bin/cmp dsk/bin/comm \
	dsk/bin/du dsk/bin/file dsk/bin/find dsk/bin/rev dsk/bin/tty \
	dsk/bin/od dsk/bin/grep dsk/bin/sort dsk/bin/tail dsk/bin/sleep \
	dsk/bin/uniq dsk/bin/make dsk/bin/stty dsk/bin/tr dsk/bin/split \
	dsk/bin/nice dsk/bin/diff dsk/bin/dc dsk/bin/bc dsk/bin/sum \
	dsk/bin/newgrp dsk/bin/lex dsk/bin/awk dsk/bin/df dsk/bin/tar \
	dsk/bin/ratfor dsk/bin/egrep dsk/bin/fgrep dsk/bin/pr dsk/bin/time \
	dsk/bin/tee dsk/bin/tp dsk/bin/ac dsk/bin/dcheck dsk/bin/icheck \
	dsk/bin/ncheck dsk/bin/clri dsk/bin/more dsk/bin/ftp dsk/bin/telnet \
	dsk/etc/glob dsk/etc/init dsk/etc/mount dsk/etc/umount dsk/etc/wall \
	dsk/etc/mknod dsk/etc/mkfs 

V6EXES = dsk/bin/login dsk/bin/mail dsk/bin/who dsk/bin/chk dsk/bin/passwd \
	dsk/bin/ps dsk/bin/man dsk/bin/nroff dsk/bin/as dsk/bin/ld \
	dsk/bin/cc dsk/lib/c0 dsk/lib/c2 dsk/bin/cdb dsk/bin/cpp dsk/bin/yacc

all: motd-${MODEL} makedirs ${EXES} ${V6EXES}

makedirs:
	@if [ ! -d dsk/bin ] ; then mkdir dsk/bin ; fi
	@if [ ! -d dsk/lib ] ; then mkdir dsk/lib ; fi
	@if [ ! -d dsk/etc ] ; then mkdir dsk/etc ; fi
	@if [ ! -d dsk/usr ] ; then mkdir dsk/usr ; fi
	@if [ ! -d dsk/usr/adm ] ; then mkdir dsk/usr/adm ; fi
	@if [ ! -d dsk/usr/include ] ; then mkdir dsk/usr/include ; fi
	@if [ ! -d dsk/usr/include/sys ] ; then mkdir dsk/usr/include/sys ; fi
	@if [ ! -d dsk/usr/lib ] ; then mkdir dsk/usr/lib ; fi
	@if [ ! -d dsk/usr/lib/term ] ; then mkdir dsk/usr/lib/term ; fi
	@if [ ! -d dsk/usr/lib/tmac ] ; then mkdir dsk/usr/lib/tmac ; fi
	@if [ ! -d dsk/usr/lib/lex ] ; then mkdir dsk/usr/lib/lex ; fi
	@if [ ! -d dsk/usr/share ] ; then mkdir dsk/usr/share ; fi
	@if [ ! -d dsk/man ] ; then mkdir dsk/man ; fi
	@if [ ! -d dsk/man/man0 ] ; then mkdir dsk/man/man0 ; fi
	@if [ ! -d dsk/man/man1 ] ; then mkdir dsk/man/man1 ; fi
	@if [ ! -d dsk/man/man2 ] ; then mkdir dsk/man/man2 ; fi
	@if [ ! -d dsk/man/man3 ] ; then mkdir dsk/man/man3 ; fi
	@if [ ! -d dsk/man/man4 ] ; then mkdir dsk/man/man4 ; fi
	@if [ ! -d dsk/man/man5 ] ; then mkdir dsk/man/man5 ; fi
	@if [ ! -d dsk/man/man6 ] ; then mkdir dsk/man/man6 ; fi
	@if [ ! -d dsk/man/man7 ] ; then mkdir dsk/man/man7 ; fi
	@if [ ! -d dsk/man/man8 ] ; then mkdir dsk/man/man8 ; fi
	@if [ ! -d dsk/man/cat0 ] ; then mkdir dsk/man/cat0 ; fi
	@if [ ! -d dsk/man/cat1 ] ; then mkdir dsk/man/cat1 ; fi
	@if [ ! -d dsk/man/cat2 ] ; then mkdir dsk/man/cat2 ; fi
	@if [ ! -d dsk/man/cat3 ] ; then mkdir dsk/man/cat3 ; fi
	@if [ ! -d dsk/man/cat4 ] ; then mkdir dsk/man/cat4 ; fi
	@if [ ! -d dsk/man/cat5 ] ; then mkdir dsk/man/cat5 ; fi
	@if [ ! -d dsk/man/cat6 ] ; then mkdir dsk/man/cat6 ; fi
	@if [ ! -d dsk/man/cat7 ] ; then mkdir dsk/man/cat7 ; fi
	@if [ ! -d dsk/man/cat8 ] ; then mkdir dsk/man/cat8 ; fi
	@if [ ! -d dsk/usr/include ] ; then mkdir dsk/usr/include ; fi
	@if [ ! -d dsk/usr/include/sys ] ; then mkdir dsk/usr/include/sys ; fi
	@if [ ! -d dsk/home ] ; then mkdir dsk/home ; fi
	@if [ ! -d dsk/home/pnr ] ; then mkdir dsk/home/pnr ; fi
	@if [ ! -d dsk/home/dgp ] ; then mkdir dsk/home/dgp ; fi
	touch dsk/usr/adm/wtmp
	cp ${INSTDIR}/usr/include/*.h dsk/usr/include/
	cp ${INSTDIR}/usr/include/sys/*.h dsk/usr/include/sys/
	cp ${INSTDIR}/lib/crt0.o dsk/lib/crt0.o
	cp ${INSTDIR}/lib/libc.a dsk/lib/libc.a
	cp ${INSTDIR}/lib/libcrt.a dsk/lib/libcrt.a
	#cp etc/fixperm dsk/etc/fixperm
	cp etc/rc dsk/etc/rc
	cp etc/motd dsk/etc/motd
	cp etc/utmp dsk/etc/utmp
	cp etc/ttys dsk/etc/ttys
	cp man/man0/* dsk/man/man0/
	cp man/man1/* dsk/man/man1/
	cp man/man2/* dsk/man/man2/
	cp man/man3/* dsk/man/man3/
	cp man/man4/* dsk/man/man4/
	cp man/man5/* dsk/man/man5/
	cp man/man6/* dsk/man/man6/
	cp man/man7/* dsk/man/man7/
	cp man/man8/* dsk/man/man8/
	cp bin/more.help dsk/usr/share/

motd-TI990 motd-TI990_10A motd-TI990_12:
	echo "***********************************" >etc/motd
	echo "*   Running Unix V6 on a TI-990   *" >>etc/motd
	echo "***********************************" >>etc/motd
	echo "" >>etc/motd

motd-TI_CTX:
	echo "**********************************" >etc/motd
	echo "* Running Unix V6 on a TI Cortex *" >>etc/motd
	echo "**********************************" >>etc/motd
	echo "" >>etc/motd

clean:
	rm -f dsk/bin/* dsk/etc/* dsk/lib/* *.o
	make -C nroff clean
	make -C yacc clean
	make -C make clean
	make -C tp clean
	make -C lex clean
	make -C awk clean
	make -C ratfor clean
	make -C ftp clean
	make -C telnet clean
	make -C ../cdb clean

dsk/% : %.c
	${TCC} ${CFLAGS} -o $@ $<

dsk/bin/cal:	bin/cal.c
dsk/bin/cat:	bin/cat.c
dsk/bin/cp:	bin/cp.c
dsk/bin/date:	bin/date.c
dsk/bin/du:	bin/du.c
dsk/bin/od:	bin/od.c
dsk/bin/file:	bin/file.c
dsk/bin/find:	bin/find.c
dsk/bin/idate:	bin/idate.c
dsk/bin/uname:	bin/uname.c
dsk/bin/touch:	bin/touch.c
dsk/bin/basename: bin/basename.c
dsk/bin/cmp:	bin/cmp.c
dsk/bin/comm:	bin/comm.c
dsk/bin/echo:	bin/echo.c
dsk/bin/ed:	bin/ed.c
dsk/bin/ln:	bin/ln.c
dsk/bin/ls:	bin/ls.c
dsk/bin/nm:	bin/nm.c
dsk/bin/mkdir:	bin/mkdir.c
dsk/bin/rmdir:	bin/rmdir.c
dsk/bin/chmod:	bin/chmod.c
dsk/bin/mv:	bin/mv.c
dsk/bin/od:	bin/od.c
dsk/bin/pwd:	bin/pwd.c
dsk/bin/rm:	bin/rm.c
dsk/bin/tr:	bin/tr.c
dsk/bin/reloc:	bin/reloc.c
dsk/bin/sh:	bin/sh.c
dsk/bin/size:	bin/size.c
dsk/bin/strip:	bin/strip.c
dsk/bin/sync:	bin/sync.c
dsk/bin/wc:	bin/wc.c
dsk/bin/kill:	bin/kill.c
dsk/bin/chgrp:	bin/chgrp.c
dsk/bin/chown:	bin/chown.c
dsk/bin/login:	bin/login.c
dsk/bin/mail:	bin/mail.c
dsk/bin/who:	bin/who.c
dsk/bin/chk:	bin/chk.c
dsk/bin/mktrace: bin/mktrace.c
dsk/bin/newgrp: bin/newgrp.c
dsk/bin/passwd: bin/passwd.c
dsk/bin/prof:	bin/prof.c
dsk/bin/ps:	bin/ps.c
dsk/bin/col:	bin/col.c
dsk/bin/man:	bin/man.c
dsk/bin/dd:	bin/dd.c
dsk/bin/rev:	bin/rev.c
dsk/bin/sum:	bin/sum.c
dsk/bin/tar:	bin/tar.c bin/hexdump.c
	$(TCC) $(CFLAGS) -o $@ bin/tar.c bin/hexdump.c
dsk/bin/sort:	bin/sort.c
dsk/bin/tail:	bin/tail.c
dsk/bin/grep:	bin/grep.c
dsk/bin/fgrep:	bin/fgrep.c
dsk/bin/time:	bin/time.c
dsk/bin/pr:	bin/pr.c
dsk/bin/uniq:	bin/uniq.c
dsk/bin/stty:	bin/stty.c
dsk/bin/sleep:	bin/sleep.c
dsk/bin/split:	bin/split.c
dsk/bin/diff:	bin/diff.c
dsk/bin/nice:	bin/nice.c
dsk/bin/dc:	bin/dc.c bin/dc.h
dsk/bin/df:	bin/df.c
dsk/bin/tee:	bin/tee.c
dsk/bin/ac:	bin/ac.c
dsk/bin/dcheck: bin/dcheck.c
dsk/bin/icheck: bin/icheck.c
dsk/bin/ncheck: bin/ncheck.c
dsk/bin/clri:	bin/clri.c
dsk/bin/more:	bin/more.c

dsk/etc/glob:	etc/glob.c
dsk/etc/init:	etc/init.c
dsk/etc/mount:	etc/mount.c
dsk/etc/umount: etc/umount.c
dsk/etc/wall:	etc/wall.c
dsk/etc/mknod:	etc/mknod.c
dsk/etc/mkfs:	etc/mkfs.c
	$(TCC) $(CFLAGS) -o $@ etc/mkfs.c etc/divrem.s

dsk/bin/nroff: 
	make -C nroff
	cp nroff/nroff $@
	cp nroff/term/tab37 dsk/usr/lib/term/
	cp nroff/tmac/* dsk/usr/lib/tmac/

dsk/bin/as:
	make -C ../as native
	cp ../as/as99.exe $@

dsk/bin/cc:
	make -C ../cc native
	cp ../cc/cc99.exe $@

dsk/bin/cpp:
	make -C ../cpp native
	cp ../cpp/cpp.exe $@

dsk/lib/c0:
	make -C ../ccom native
	cp ../ccom/c0.exe dsk/lib/c0
	cp ../ccom/c1.exe dsk/lib/c1

dsk/bin/ld:
	make -C ../binutils native
	cp ../binutils/ld.exe dsk/bin/ld
	cp ../binutils/ar.exe dsk/bin/ar

dsk/lib/c2:
	make -C ../c2 native
	cp ../c2/c2.exe $@

dsk/bin/cdb:
	make -C ../cdb 
	cp ../cdb/cdb $@

dsk/bin/yacc:
	make -C yacc native
	cp yacc/yacc.exe $@
	cp yacc/yaccpar dsk/usr/lib/

dsk/bin/make: 
	make -C make
	cp make/make $@

dsk/bin/tp: 
	make -C tp
	cp tp/tp $@

dsk/bin/awk: 
	make -C awk
	cp awk/awk $@

dsk/bin/ratfor: 
	make -C ratfor
	cp ratfor/ratfor $@

dsk/bin/ftp: 
	make -C ftp
	cp ftp/ftp $@

dsk/bin/telnet: 
	make -C telnet
	cp telnet/telnet $@

dsk/bin/bc: bin/bc.c
bin/bc.c: bin/bc.y
	${YACC} bin/bc.y
	mv y.tab.c bin/bc.c

dsk/bin/egrep: bin/egrep.c
bin/egrep.c: bin/egrep.y
	${YACC} bin/egrep.y
	mv y.tab.c bin/egrep.c

dsk/bin/lex: 
	make -C lex native
	cp lex/lex $@
	cp lex/libln.a dsk/lib/
	cp lex/ncform dsk/usr/lib/lex/

