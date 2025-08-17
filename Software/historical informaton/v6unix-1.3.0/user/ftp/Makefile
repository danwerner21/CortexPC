TCC = ~/cc99/bin/cc99

.c.o:
	${TCC} -O -c -DKERNEL -I. -I../../bbnnet -o $@ $<

all:	ftp 

FTPOBJS = ftp.o die.o tcp_usr.o tcp_fns.o ftp_lib.o

ftp: ${FTPOBJS}
	${TCC} -a 0x1000 -o ftp $(FTPOBJS) -L../../bbnnet -lnet

clean:
	rm -f ${FTPOBJS} ftp

ftp.o: ftp.c ftp.h usr.h ftptelnet.h ftp_lib.h
tcp_fns.o: tcp_fns.c ftp.h
tcp_usr.o: tcp_usr.c ftp.h usr.h
ftp_lib.o: ftp_lib.c ftp_lib.h ftp.h
die.o: die.c
