INSTDIR	= /usr/local

all:
	@if [ ! -d ${INSTDIR}/usr ] ; then mkdir ${INSTDIR}/usr ; fi
	@if [ ! -d ${INSTDIR}/usr/include ] ; then mkdir ${INSTDIR}/usr/include ; fi
	@if [ ! -d ${INSTDIR}/usr/include/sys ] ; then mkdir ${INSTDIR}/usr/include/sys ; fi

install:
	cp *.h ${INSTDIR}/usr/include/
	cp sys/*.h ${INSTDIR}/usr/include/sys/

clean:
