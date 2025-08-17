
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <utmp.h>
#include <sgtty.h>
#include <sys/stat.h>

/*
 * login [ name ]
 */

struct sgttyb ttyb;
struct stat statb;
struct utmp utmp;

char	*ttyx;
char	*colon();
char	*crypt();

main(argc, argv)
	char **argv;
{
	char pbuf[128];
	register char *namep, *np;
	char pwbuf[9];
	int t, sflags, f, c, uid, gid;
	int me;

	signal(SIGQUIT, 1);
	signal(SIGINT, 1);

	nice(0);
	ttyx = "/dev/ttyx";
	if ((me=ttyn(0)) == 'x') {
		write(1, me, 1);
		write(1, ": Sorry.\n", 9);
		exit();
	}
	ttyx[8] = me;

	/* Restore terminal (if changed) */
	gtty (0, &ttyb);
	ttyb.sg_ispeed = ttyb.sg_ospeed = B9600;
	ttyb.sg_flags = XTABS | ECHO | CRMOD;
	stty (0, &ttyb);

	/* Reset (clear) utmp entry */
	memset (&utmp, 0, sizeof (utmp));
	utmp.ut_line[0] = 't';
	utmp.ut_line[1] = me;
	time(&utmp.ut_time);
	if ((f = open("/etc/utmp", 1)) >= 0) {
		t = me;
		if (t>='a')
			t -= 'a' - (10+'0');
		lseek(f, (long)((t-'0')*sizeof(utmp)), 0);
		write(f, &utmp, sizeof(utmp));
		close(f);
	}
	if ((f = open("/usr/adm/wtmp", 1)) >= 0) {
		seek(f, 0, 2);
		write(f, &utmp, sizeof(utmp));
		close(f);
	}

    loop:
	namep = utmp.ut_name;

	if (argc>1) {
		np = argv[1];
		while (namep<utmp.ut_name+8 && *np)
			*namep++ = *np++;
		argc = 0;
	} else {
		write(1, "\nlogin: ", 8);
		while ((c = getchar()) != '\n') {
			if (c <= 0)
				exit();
			if (namep < utmp.ut_name+8)
				*namep++ = c;
		}
	}
	if (namep == utmp.ut_name)
		goto loop;
	while (namep < utmp.ut_name+8)
		*namep++ = ' ';
	if (getpwentry(utmp.ut_name, pbuf)) {
		write(1, "getpwentry fail.\n", 17);
		goto bad;
	}
	np = colon(pbuf);
	if (*np!=':') {
		sflags = ttyb.sg_flags;
		ttyb.sg_flags &= ~ECHO;
		stty(0, &ttyb);
		write(1, "Password: ", 10);
		namep = pwbuf;
		while ((c=getchar()) != '\n') {
			if (c <= 0)
				exit();
			if (namep<pwbuf+8)
				*namep++ = c;
		}
		*namep++ = '\0';
		ttyb.sg_flags = sflags;
		stty(0, &ttyb);
		write(1, "\n", 1);
		namep = crypt(pwbuf);
		while (*namep++ == *np++);
		if (*--namep!='\0' || *--np!=':') {
			write(1, "crypt incorrect.\n", 17);
			goto bad;
		}
	}
	np = colon(np);
	uid = 0;
	while (*np != ':')
		uid = uid*10 + *np++ - '0';
	np++;
	gid = 0;
	while (*np != ':')
		gid = gid*10 + *np++ - '0';
	np++;
	np = colon(np);
	namep = np;
	np = colon(np);
	if (chdir(namep)<0) {
		write(1, "No directory\n", 13);
		goto loop;
	}

	time(&utmp.ut_time);
	if ((f = open("/etc/utmp", 1)) >= 0) {
		t = utmp.ut_line[1];
		if (t>='a')
			t -= 'a' - (10+'0');
		lseek(f, (long)((t-'0')*sizeof(utmp)), 0);
		write(f, &utmp, sizeof(utmp));
		close(f);
	}
	if ((f = open("/usr/adm/wtmp", 1)) >= 0) {
		seek(f, 0, 2);
		write(f, &utmp, sizeof(utmp));
		close(f);
	}
	if ((f = open("/etc/motd", 0)) >= 0) {
		while(read(f, &t, 1) > 0)
			write(1, &t, 1);
		close(f);
	}
	if(stat(".mail", &statb) >= 0 && statb.st_size)
		write(1, "You have mail.\n", 15);

	chown(ttyx, uid);
	setgid(gid);
	setuid(uid);
	if (*np == '\0')
		np = "/bin/sh";
	execl(np, "sh", 0);
	write(1, "No shell.\n", 9);
	exit();
bad:
	write(1, "Login incorrect.\n", 17);
	goto loop;
}

int
getpwentry(name, buf)
	char *name, *buf;
{
	int r, c;
	FILE *fp;
	register char *gnp, *rnp;

	r = 1;
	if((fp = fopen("/etc/passwd", "r")) < 0)
		goto ret;
loop:
	gnp = name;
	rnp = buf;
	while((c=getc(fp)) != '\n') {
		if(c <= 0)
			goto ret;
		*rnp++ = c;
	}
	*rnp++ = '\0';
	rnp = buf;
	while (*gnp++ == *rnp++);
	if ((*--gnp!=' ' && gnp<name+8) || *--rnp!=':')
		goto loop;
	r = 0;
ret:
	fclose(fp);
	return(r);
}

char*
colon(p)
	char *p;
{
	register char *rp;

	rp = p;
	while (*rp != ':') {
		if (*rp++ == '\0') {
			write(1, "Bad /etc/passwd\n", 16);
			exit();
		}
	}
	*rp++ = '\0';
	return(rp);
}
