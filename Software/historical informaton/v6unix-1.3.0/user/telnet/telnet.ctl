NOSYMT
PROCEDURE NETCMN
DUMMY
INCLUDE ../oct82/netcmn.o
TASK TELNET
INCLUDE /usr/local/lib/gcc/ti990-ti-dx10/3.4.6/crt0.o
;INCLUDE /usr/local/lib/gcc/ti990-ti-dx10/3.4.6/crt0os.o
INCLUDE telnet.o
INCLUDE process.o
INCLUDE cmds.o
INCLUDE options.o
INCLUDE args.o
INCLUDE ttyctl.o
INCLUDE ../oct82/netuser.o
INCLUDE ../oct82/host.o
INCLUDE ../oct82/mbuf.o
INCLUDE ../oct82/sema.o
INCLUDE ../oct82/usr.o
FIND libtn.a
FIND ../oct82/libnet.a
FIND /usr/local/lib/gcc/ti990-ti-dx10/3.4.6/libcsys.a
;FIND /usr/local/lib/gcc/ti990-ti-dx10/3.4.6/libsoft.a
FIND /usr/local/lib/gcc/ti990-ti-dx10/3.4.6/libsci.a
END
