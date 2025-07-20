TOOLS	= cc user/yacc cpp ccom c2 as binutils libc/include fsutil user/lex

INSTDIR	= ~/cc99

MODEL   = TI990
CONSOLE = TTYEIA
MAXSIZE = 33024000

#MODEL   = TI_CTX
#CONSOLE = TTY9902
#MAXSIZE = 3670016

TARGETS = makedirs tools tools-inst

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


darwin:
	@make $(TARGETS) "CC_CFLAGS = -DUNIX -DDARWIN $(DEBUG)" \
 		"YACC = byacc"

linux:
	@make $(TARGETS) "CC_CFLAGS = -DUNIX -DLINUX $(DEBUG)"


tools: makedirs
	for i in ${TOOLS}; do $(MAKE) INSTDIR=$(INSTDIR) -C $$i; done

tools-inst:
	for i in ${TOOLS}; do $(MAKE) INSTDIR=$(INSTDIR) -C $$i install; done

install: tools-inst

makedirs:
	mkdir -p ${INSTDIR}/bin ${INSTDIR}/lib ${INSTDIR}/usr

clean:
#	for i in ${SUBDIR}; do $(MAKE) -C $$i clean; done

distclean: clean
	for i in ${TOOLS}; do $(MAKE) -C $$i clean; done
	rm -f make.log
