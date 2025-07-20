#
# Make system call part of std C library.
#
INSTDIR	= /usr/local
MODEL	= TI990
CONSOLE = TTYEIA
#MODEL	= TI_CTX
#CONSOLE = TTY9902

AS = ${INSTDIR}/bin/as99
AR = ${INSTDIR}/bin/ar99

OBJS = chdir.o close.o creat.o dup.o execv.o exit.o gtty.o \
      fork.o fstat.o getpid.o setuid.o getuid.o open.o pipe.o \
      read.o mknod.o getgid.o chown.o chmod.o stime.o sync.o \
      reset.o seek.o signal.o stat.o time.o times.o wait.o \
      write.o break.o execl.o link.o unlink.o cerror.o kill.o \
      ptrace.o bground.o setgid.o nice.o tsleep.o sleep.o stty.o mount.o \
      profil.o uname.o dup2.o utime.o sysnet.o ioctl.o netopen.o

all: syscall

install: all
	${AR} r ${INSTDIR}/lib/libc.a ${OBJS}

clean:
	rm -f *.o

syscall: ${OBJS}

.s.o:
	${AS} -u -o $@ ../../v6/${MODEL}-${CONSOLE}.cfg ../../v6/hdr.s $<

cerror.o: cerror.s
execv.o: execv.s
execl.o: execl.s
getuid.o: getuid.s
seek.o: seek.s
wait.o: wait.s
chdir.o: chdir.s
exit.o: exit.s
gtty.o: gtty.s
open.o: open.s
signal.o: signal.s
rite.o: rite.s
close.o: close.s
fork.o: fork.s
pipe.o: pipe.s
stat.o: stat.s
creat.o: creat.s
fstat.o: fstat.s
read.o: read.s
time.o: time.s
dup.o: dup.s
dup2.o: dup2.s
getpid.o: getpid.s
setuid.o: setuid.s
reset.o: reset.s
times.o: times.s
break.o: break.s
link.o: link.s
unlink.o: unlink.s
mknod.o: mknod.s
getgid.o: getgid.s
chown.o: chown.s
chmod.o: chmod.s
stime.o: stime.s
sync.o: sync.s
kill.o: kill.s
ptrace.o: ptrace.s
bground.o: bground.s
setgid.o: setgid.s
stty.o: stty.s
nice.o: nice.s
sleep.o: sleep.s
msleep.o: msleep.s
mount.o: mount.s
profil.o: profil.s
uname.o: uname.s
utime.o: utime.s
sysnet.o: sysnet.s
ioctl.o: ioctl.s
netopen.o: netopen.s
