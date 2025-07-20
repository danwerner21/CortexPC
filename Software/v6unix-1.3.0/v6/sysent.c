
#include "param.h"
#include "systm.h"

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 * Each row contains the number of arguments expected
 * and a pointer to the routine.
 */
struct sysent sysent[64] =
{
	0, &nullsys,			/*  0 = indir */
	1, &rexit,			/*  1 = exit */
	0, &fork,			/*  2 = fork */
	3, &read,			/*  3 = read */
	3, &write,			/*  4 = write */
	2, &open,			/*  5 = open */
	1, &close,			/*  6 = close */
	0, &wait,			/*  7 = wait */
	2, &creat,			/*  8 = creat */
	2, &link,			/*  9 = link */
	1, &unlink,			/* 10 = unlink */
	2, &exec,			/* 11 = exec */
	1, &chdir,			/* 12 = chdir */
	1, &gtime,			/* 13 = time */
	3, &mknod,			/* 14 = mknod */
	2, &chmod,			/* 15 = chmod */
	2, &chown,			/* 16 = chown */
	1, &sbreak,			/* 17 = break */
	2, &stat,			/* 18 = stat */
	4, &lseek,			/* 19 = seek */
	0, &getpid,			/* 20 = getpid */
	3, &smount,			/* 21 = mount */
	1, &sumount,			/* 22 = umount */
	1, &setuid,			/* 23 = setuid */
	0, &getuid,			/* 24 = getuid */
	1, &stime,			/* 25 = stime */
	4, &ptrace,			/* 26 = ptrace */
	1, &utsinfo,			/* 27 = uname */
	2, &fstat,			/* 28 = fstat */
	1, &nosys,			/* 29 = x */
	2, &utime,			/* 30 = utime */
	2, &stty,			/* 31 = stty */
	2, &gtty,			/* 32 = gtty */
	0, &nosys,			/* 33 = x */
	1, &nice,			/* 34 = nice */
	1, &sslep,			/* 35 = sleep */
	0, &sync,			/* 36 = sync */
	2, &kill,			/* 37 = kill */
	0, &getswit,			/* 38 = switch */
	0, &nosys,			/* 39 = x */
	2, &dup2,			/* 40 = dup2 */
	1, &dup,			/* 41 = dup */
	2, &pipe,			/* 42 = pipe */
	1, &times,			/* 43 = times */
	4, &profil,			/* 44 = prof */
	5, &sysnet,			/* 45 = sysnet */
	1, &setgid,			/* 46 = setgid */
	0, &getgid,			/* 47 = getgid */
	2, &ssig,			/* 48 = sig */
	3, &sioctl,			/* 49 = ioctl */
	1, &netopen,			/* 50 = netopen */
	0, &nosys,			/* 51 = x */
	0, &nosys,			/* 52 = x */
	0, &nosys,			/* 53 = x */
	0, &nosys,			/* 54 = x */
	0, &nosys,			/* 55 = x */
	0, &nosys,			/* 56 = x */
	0, &nosys,			/* 57 = x */
	0, &nosys,			/* 58 = x */
	0, &nosys,			/* 59 = x */
	0, &nosys,			/* 60 = x */
	0, &nosys,			/* 61 = x */
	0, &nosys,			/* 62 = x */
	0, &nosys			/* 63 = x */
};
