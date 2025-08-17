TOOLS	= cc user/yacc cpp ccom c2 as binutils libc/include user/lex
RTLDIR	= crt libc/syscall libc/stdio libc/std
OSDIR 	= v6 bbnnet
USRDIR	= user
DSKDIR	= user/dsk
SUBDIR	= ${RTLDIR} ${OSDIR} ${USRDIR} ${DSKDIR}

INSTDIR	= ~/cc99

MODEL   = TI990
CONSOLE = TTYEIA
MAXSIZE = 33024000

#MODEL   = TI_CTX
#CONSOLE = TTY9902
#MAXSIZE = 3670016

TARGETS = rtl rtl-inst os usr dsk

all: 
	@if [ "`uname -s`" = "Linux" ] ; then \
		echo "Making Linux on a `uname -m`" ;\
		make linux ;\
	elif [ "`uname -s`" = "Darwin" ] ; then \
		echo "Making Darwin (OS X)" ;\
		make darwin ;\
	else \
		echo "OS type `uname -s` is unknown" ;\
		echo "You must enter an OS type. OS types are:" ;\
		echo "   darwin | linux " ;\
		echo " " ;\
	fi


darwin: tools
	@make $(TARGETS) "CC_CFLAGS = -DUNIX -DDARWIN $(DEBUG)" 

linux: tools
	@make $(TARGETS) "CC_CFLAGS = -DUNIX -DLINUX $(DEBUG)"


tools:
	$(MAKE) INSTDIR=$(INSTDIR) -C host

rtl:
	for i in ${RTLDIR}; do $(MAKE) INSTDIR=$(INSTDIR) -C $$i; done
	$(MAKE) INSTDIR=$(INSTDIR) -C bbnnet libnet.a

os:
	for i in ${OSDIR}; do $(MAKE) MODEL=${MODEL} CONSOLE=${CONSOLE} INSTDIR=$(INSTDIR) -C $$i; done

usr:
	for i in ${USRDIR}; do $(MAKE) MODEL=${MODEL} INSTDIR=$(INSTDIR) -C $$i; done

rtl-inst:
	rm -f ${INSTDIR}/lib/libc.a
	rm -f ${INSTDIR}/lib/liba.a
	for i in ${RTLDIR}; do $(MAKE) MODEL=${MODEL} INSTDIR=$(INSTDIR) -C $$i install; done
	$(MAKE) INSTDIR=$(INSTDIR) -C bbnnet install

install: tools rtl-inst

dsk:
	for i in ${DSKDIR}; do $(MAKE) MODEL=${MODEL} MAXSIZE=${MAXSIZE} INSTDIR=${INSTDIR} -C $$i; done

clean:
	$(MAKE) -C host clean
	for i in ${SUBDIR}; do $(MAKE) -C $$i clean; done

distclean: clean
	$(MAKE) -C host distclean
	for i in ${TOOLS}; do $(MAKE) -C $$i clean; done
	rm -f make.log
