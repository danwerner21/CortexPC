#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <utmp.h>

#define MAXTTY 8
#define MAXDISK 4
#define MAXPARTITION 16

struct mtab {
	char file[32];
	char spec[32];
};

static char shell[]	= "/bin/sh";
static char runc[]	= "/etc/rc";
static char init[]	= "/etc/init";
static char ttys[]	= "/etc/ttys";
static char ctty[]	= "/dev/tty0";

static struct {
	char  name[8];
	int   type;
	char *value;
} nl[4];

static struct mtab mtable;

static int  ttypid[MAXTTY];
static char ttynum[MAXTTY];
static struct utmp utmp;
#ifndef TI_CTX
static int partition[MAXPARTITION+1];
static int disks[MAXDISK];
#endif

int
startgtty(tty)
	char *tty;
{
	int pid;

	pid = fork();
	if (pid == 0) {
		close(0);
		open(tty, 2);
		dup(0);
		dup(0);
		execl("/bin/login", "login", 0);
		exit();
	}
	return (pid);
}

main()
{
	FILE *ffd;
	register i, j, max;
	register struct tab *p, *q;
	int mtablen;
	int fd, devno;
	int reset();
	char devname[32];
	char line[32];

	/*
	 * Set up /etc/mtab for the root device.
	 */
	strcpy (nl[0].name, "_rootdev");
#ifndef TI_CTX
	strcpy (nl[1].name, "_partiti");
	strcpy (nl[2].name, "_disks");
#else
	nl[1].name[0] = 0;
#endif
	nl[3].name[0] = 0;

	nlist ("/unix", nl);
	if (nl[0].type != 0) {
	   devno = 0;
	   if ((fd = open ("/dev/mem", 0)) >= 0) {
	      lseek (fd, (long)(nl[0].value), 0);
	      read (fd, &devno, 2);
	      close (fd);
	      if ((fd = creat ("/etc/mtab", 0644)) >= 0) {
		 sprintf (mtable.spec, "dsk%d", devno >> 12);
		 strcpy (mtable.file, "/");
		 write (fd, &mtable, sizeof(struct mtab));
		 close (fd);
	      }
	   }
	}

#ifndef TI_CTX
	/*
	 * Set up partition device names on online disks.
	 */
	if (nl[1].type != 0 && nl[2].type != 0) {
	   if ((fd = open ("/dev/mem", 0)) >= 0) {
	      lseek (fd, (long)(nl[1].value), 0);
	      read (fd, &partition, sizeof (partition));
	      lseek (fd, (long)(nl[2].value), 0);
	      read (fd, &disks, sizeof (disks));
	      close (fd);
	      for (j = 0; j < MAXDISK; j++) {
		 if (disks[j] == 0) {
		    sprintf (devname, "/dev/dsk%d", j);
		    unlink (devname);
	  	 } else for (i = 1; i < MAXPARTITION; i++) {
		    if (partition[i] < 0) break;
		    sprintf (devname, "/dev/dsk%dp%d", j, i);
		    mknod (devname, 060644, (j << 12) | (i << 8));
		 }
	      }
	   }
	}
#endif

	/*
	 * Run initialize date/time
	 */
	i = fork();
	if(i == 0) {
		open(ctty, 0);
		open(ctty, 1);
		dup(0);
		execl("/bin/idate", 0);
		exit();
	}
	while(wait() != i);

	/*
	 * run boot script
	 */
	i = fork();
	if(i == 0) {
		open("/", 0);
		open(ctty, 1);
		dup(0);
		execl(shell, shell, runc, 0);
		exit();
	}
	while(wait() != i);

	/* log reboot in wtmp */
	memset (&utmp, 0, sizeof (utmp));
	utmp.ut_line[0] = '~';
	utmp.ut_line[1] = 0;
	time(&utmp.ut_time);
	if ((fd = open("/usr/adm/wtmp", 1)) >= 0) {
		seek(fd, 0, 2);
		write(fd, &utmp, sizeof(utmp));
		close(fd);
	}

	/*
	 * main loop for hangup signal
	 * start shell on each tty & restart forever
	 */

	setexit();
	signal(1, reset);

	/* Determine how many ttys we have and set up array */
	if ((ffd = fopen (ttys, "r")) != NULL)
	{
		for (max = 0; max < MAXTTY; ) {
			if (fgets (line, 32, ffd) == NULL)
				break;
			if (line[0] == '1') {
				ttynum[max] = line[1];
				max++;
			}
		}
		fclose (ffd);
	}

	/* Start a shell for each specified terminal */
	for (j = 0; j < max; j++) {
		strcpy (devname, "/dev/ttyx");
		devname[8] = ttynum[j];
		ttypid[j] = startgtty(devname);
	}

	/* Wait for a shell to terminate and restart it */
	for(;;) {
		i = wait();
		for (j = 0; j < max; j++) {
			if (ttypid[j] == i) {
				strcpy (devname, "/dev/ttyx");
				devname[8] = ttynum[j];
				ttypid[j] = startgtty(devname);
			}
		}
	}
}
