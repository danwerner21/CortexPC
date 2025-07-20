TCC = ~/cc99/bin/cc99

.c.o:
	${TCC} -O -c -DKERNEL -I../../bbnnet -o $@ $<

TELOBJS = telnet.o process.o cmds.o options.o args.o ttyctl.o \
	tel.o netc.o atoiv.o cmderr.o

all:	telnet

telnet: $(TELOBJS)
	${TCC} -a 0x1000 -o telnet $(TELOBJS) -L../../bbnnet -lnet

clean:
	rm -f ${TELOBJS} telnet telnet.map

cmds.o: cmds.c tnio.h telnet.h globdefs.h ttyctl.h
options.o: options.c globdefs.h tnio.h telnet.h ttyctl.h
process.o: process.c tnio.h globdefs.h ttyctl.h
ttyctl.o: ttyctl.c ttyctl.h
telnet.o: telnet.c globdefs.h tnio.h telnet.h ttyctl.h

atoiv.o: atoiv.c
cmderr.o: cmderr.c
netc.o: netc.c tnio.h telnet.h
tel.o: tel.c tnio.h telnet.h
