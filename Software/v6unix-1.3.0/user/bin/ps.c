
/*
 *	ps - process status
 *	examine and print certain things about processes
 */

#include "param.h"
#include "proc.h"
#include "tty.h"
#include "user.h"

#undef NULL

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

struct {
	char  name[8];
	int   type;
	char *value;
} nl[3];

struct proc proc[NPROC];
struct tty  tty;
struct user u;

int	lflg;
int	kflg;
int	xflg;
int	aflg;
int	mem;
int	swap;

int	stbuf[257];
int	ndev;
char	devc[65];
int	devl[65];
struct tty *devt[65];
char	*coref;


main(argc, argv)
	char **argv;
{
	struct proc *p;
	int n, b;
	int i, c, mtty;
	char *ap;
	int uid, puid;

	if (argc>1) {
		ap = argv[1];
		while (*ap) switch (*ap++) {
		case 'a':	/* all processes with a tty*/
			aflg++;
			break;

		case 'x':	/* also include with no tty */
			xflg++;
			break;

		case 'l':	/* long output format */
			lflg++;
			break;

		case 'k':	/* use /usr/sys/core instead of /dev/mem */
			kflg++;
			break;

		}
	}

	setuid (0);
	if(chdir("/dev") < 0) {
		printf("cannot change to /dev\n");
		done();
	}
	setup(&nl[0], "_proc");
	setup(&nl[1], "_swapdev");
	nlist(argc>2? argv[2]:"/unix", nl);
	if (nl[0].type==0) {
		printf("No namelist\n");
		return;
	}
	coref = "/dev/mem";
	if(kflg)
		coref = "/usr/bin/core";
	if ((mem = open(coref, 0)) < 0) {
		printf("No mem\n");
		done();
	}
	lseek(mem, (long)(nl[1].value), 0);
	read(mem, &nl[1].value, 2);
	lseek(mem, (long)(nl[0].value), 0);
	read(mem, proc, sizeof proc);
	getdev();
	mtty = ttyn(0);
	uid = getuid() & 0377;

	if(lflg)
		printf("TTY F S UID   PID PRI SLOT  SZ  WCHAN COMMAND\n");
	else
		printf("TTY  PID COMMAND\n");

	for (i=0; i<NPROC; i++) {
		if (proc[i].p_stat==0)
			continue;
		if (proc[i].p_ttyp==0) {
			if (xflg==0)
				continue;
			c = '?';
		} else {
			for(c=0; c<ndev; c++)
				if(devt[c] == proc[i].p_ttyp) {
					c = devc[c];
					goto out;
				}
			lseek(mem, (long)(proc[i].p_ttyp), 0);
			read(mem, &tty, sizeof tty);
			for(c=0; c<ndev; c++)
				if(devl[c] == tty.t_dev) {
					devt[c] = proc[i].p_ttyp;
					c = devc[c];
					goto out;
				}
			c = '?';
		out:;
		}
		puid = proc[i].p_uid & 0377;
		if (uid != puid && aflg==0)
			continue;
		printf("%c:", c);
		if (lflg) {
			printf("%3o %c%4d", proc[i].p_flag,
				"0SWRIZT"[proc[i].p_stat], puid);
		}
		printf("%6d", proc[i].p_pid);
		if (lflg) {
			if (proc[i].p_slot < CMAPSIZ)
				printf("%4d%5d%4d",
					proc[i].p_pri, proc[i].p_slot, 64);
			else
				printf("%4d%5x%4d",
					proc[i].p_pri, proc[i].p_slot, 64);
			if (proc[i].p_wchan)
				printf("%7x", proc[i].p_wchan);
			else
				printf("       ");
		}
		if (proc[i].p_stat==5)
			printf(" <defunct>");
		else
			prcom(i);
		printf("\n");
	}
	done();
}

getdev()
{
	register struct dirent *p;
	register int i, c;
	int f;
	char dbuf[512];
	struct stat sbuf;

	f = open("/dev", 0);
	if(f < 0) {
		printf("cannot open /dev\n");
		done();
	}

	swap = -1;
	c = 0;
loop:
	i = read(f, dbuf, 512);
	if(i <= 0) {
		close(f);
		if(swap < 0) {
			printf("no swap device\n");
			done();
		}
		ndev = c;
		return;
	}
	while(i < 512)
		dbuf[i++] = 0;

	for(p = (struct dirent*)dbuf; p < (struct dirent*)(dbuf+512); p++) {
		if(p->d_ino == 0)
			continue;
		if(p->d_name[0] == 't' &&
		   p->d_name[1] == 't' &&
		   p->d_name[2] == 'y' &&
		   p->d_name[4] == 0 &&
		   p->d_name[3] != 0) {
			if(stat(p->d_name, &sbuf) < 0)
				continue;
			devc[c] = p->d_name[3];
			devl[c] = sbuf.st_addr[0];
			c++;
			continue;
		}
		if(swap >= 0)
			continue;
		if(stat(p->d_name, &sbuf) < 0)
			continue;
		if((sbuf.st_mode & S_IFMT) != S_IFBLK)
			continue;
		if(sbuf.st_addr[0] == nl[1].value)
			swap = open(p->d_name, 0);
	}
	goto loop;
}

setup(p, s)
char *p, *s;
{
	while (*p++ = *s++);
}

int
prcom(i)
{
	int mf;
	long offs;
	register int *ip;
	register char *cp, *cp1;
	int c, nbad;

	if(i==0) {
		printf(" [swapper]");
		return(1);
	}

	if (proc[i].p_flag&SLOAD) {
		offs = proc[i].p_slot;
		offs <<= 16;
		mf = mem;
	} else {
		offs = proc[i].p_slot;
		offs <<= 9;
		mf = swap;
	}
	offs += 0xee00L;
	lseek(mf, offs, 0);
	if (read(mf, stbuf, 512) != 512) {
		printf("bad read");
		return(0);
	}

	/* scan stack for begin of arguments character block */
	for (ip = &stbuf[255]; ip > &stbuf[0];) {
		if (*--ip == 0) {
			cp = (char*)(ip+1);
			if (*cp==0)
				cp++;
			nbad = 0;
			for (cp1 = cp; cp1 < &stbuf[256]; cp1++) {
				c = *cp1;
				if (c==0)
					*cp1 = ' ';
				else if (c < ' ' || c > 0176) {
					if (++nbad >= 5) {
						*cp1++ = ' ';
						break;
					}
					*cp1 = '?';
				}
			}
			while (*--cp1==' ')
				*cp1 = 0;
			printf(lflg?" %.36s":" %.64s", cp);
			return(1);
		}
	}
	return(0);
}

done()
{
	fflush(stdout);
	exit(0);
}


