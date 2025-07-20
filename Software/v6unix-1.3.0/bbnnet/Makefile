INSTDIR=~/cc99
TCC=${INSTDIR}/bin/cc99
AS=${INSTDIR}/bin/as99
NM=${INSTDIR}/bin/nm99
AR=${INSTDIR}/bin/ar99

.c.o:
	${TCC} -O -c -I. -I../v6 -DKERNEL ${DEBUG} -o $@ $<
.s.o:
	${AS} -u -o $@ $<

NETOBJS = netsupport.o host.o icmp.o ip_input.o ip_output.o if_lo.o mbuf.o \
	netconf.o raw_input.o if_slip.o sysdep.o raw_input.o tcp_input.o \
	tcp_prim.o tcp_procs.o tcp_states.o udp.o netuser.o printf.o
LIBOBJS = inet_addr.o inet_ntoa.o gethost.o errmsg.o hostname.o thisname.o

LIBS = libnet.a

EXES = net netstat tcptest nettest netserv

all: ${LIBS} ${EXES}

net: ${NETOBJS} main.o netcmn.o
	${TCC} -a 0x1000 -o $@ main.o ${NETOBJS} netcmn.o -L. -lnet
	$(NM) -ng net >net.map

netstat: netstat.o libnet.a
	${TCC} -a 0x1000 -o $@ netstat.o -L. -lnet

tcptest: tcptest.o libnet.a
	${TCC} -a 0x1000 -o $@ tcptest.o -L. -lnet

nettest: nettest.o libnet.a
	${TCC} -a 0x1000 -o $@ nettest.o -L. -lnet

netserv: netserv.o libnet.a
	${TCC} -a 0x1000 -o $@ netserv.o -L. -lnet

libnet.a: ${LIBOBJS}
	${AR} r $@ ${LIBOBJS}

clean:
	rm -f net.map main.o
	rm -f ${EXES} tcptest.o netstat.o netcmn.o
	rm -f nettest.o netserv.o
	rm -f ${NETOBJS} *.lst
	rm -f ${LIBOBJS} libnet.a

install: ${LIBOBJS}
	${AR} r ${INSTDIR}/lib/libnet.a ${LIBOBJS}

host.o: host.c net.h mbuf.h
icmp.o: icmp.c net.h mbuf.h ip.h ifcb.h ucb.h tcp.h raw.h icmp.h
ip_input.o: ip_input.c net.h mbuf.h ip.h ifcb.h udp.h tcp.h icmp.h con.h
ip_output.o: ip_output.c net.h mbuf.h ip.h ifcb.h ucb.h raw.h
mbuf.o: mbuf.c net.h mbuf.h ip.h ifcb.h ucb.h
if_lo.o: if_lo.c net.h mbuf.h ip.h ifcb.h
netcmn.o: netcmn.c net.h mbuf.h ip.h ifcb.h ucb.h tcp.h fsm.h
netconf.o: netconf.c net.h mbuf.h ip.h ifcb.h
raw_input.o: raw_input.c net.h mbuf.h ip.h ifcb.h ucb.h tcp.h raw.h udp.h icmp.h
if_slip.o: if_slip.c net.h mbuf.h ip.h ifcb.h
tcp_input.o: tcp_input.c net.h mbuf.h ip.h ifcb.h ucb.h tcp.h fsm.h fsmdef.h
tcp_prim.o: tcp_prim.c net.h mbuf.h ip.h ifcb.h ucb.h tcp.h fsm.h
tcp_procs.o: tcp_procs.c net.h mbuf.h ip.h ifcb.h ucb.h tcp.h fsm.h
tcp_states.o: tcp_states.c net.h mbuf.h ip.h ifcb.h ucb.h tcp.h fsm.h
udp.o: udp.c net.h mbuf.h ip.h ifcb.h ucb.h udp.h

netsupport.o: netsupport.s
main.o: main.c net.h mbuf.h ip.h ifcb.h ucb.h tcp.h fsm.h
netuser.o: netuser.c net.h mbuf.h ip.h ifcb.h ucb.h tcp.h fsm.h
printf.o: printf.c
sysdep.o: sysdep.c

netstat.o: netstat.c
nettest.o: nettest.c
netserv.o: netserv.c

inet_addr.o: inet_addr.c
inet_ntoa.o: inet_ntoa.c
gethost.o: gethost.c
errmsg.o: errmsg.c
hostname.o: hostname.c
thisname.o: thisname.c
