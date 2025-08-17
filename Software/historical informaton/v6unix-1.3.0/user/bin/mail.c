#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <utmp.h>

/* mail command usage
	mail
		prints your mail
	mail people
		sends standard input to people
 */

#define	SIGINT	2

struct	passwd {
	char	*pw_name;
	char	*pw_passwd;
	int	pw_uid;
	int	pw_gid;
	char	*pw_gecos;
	char	*pw_dir;
	char	*pw_shell;
};

char	lettmp[]  = "/tmp/maxxxxx";
char	preptmp[] = "/tmp/mbxxxxx";
FILE*	pwfil;
FILE*	fout;

struct passwd* getpwent();
void printmail();
char *pwskip();
char *cat();

main(argc, argv)
	char **argv;
{
	register int me;
	register struct passwd *p;
	register char *cp;
	static struct utmp ubuf;
	int uf;

	maketemp();
	if (argc==1 || argc==2 && argv[1][0]=='-') {
		printmail(argc, argv);
		delexit();
	}
	signal(SIGINT, delexit);
	fout = fopen(lettmp, "w");
	if (((me=ttyn(1))!='x' || (me=ttyn(2))!='x')
	    && (uf = open("/etc/utmp", 0)) > 0) {
		while (read(uf, &ubuf, sizeof ubuf) == sizeof ubuf)
			if (ubuf.ut_line[1] == me) {
				ubuf.ut_name[8] = ' ';
				close(uf);
				for (cp=ubuf.ut_name; *cp++!=' ';);
				*--cp = 0;
				bulkmail(argc, argv, ubuf.ut_name);
			}
	}
	me = getuid() & 0377;
	setpw();
	for (;;)
		if ((p = getpwent()) && p->pw_uid == me)
			bulkmail(argc, argv, p->pw_name);
	printf("Who are you?\n");
	delexit();
}

void
printmail(argc, argv)
	char **argv;
{
	FILE *fin;
	register n, c, f;

	if ((fin = fopen(".mail", "r")) && (c = getc(fin))>=0) {
		do {
			putchar(c);
		} while ((c = getc(fin))>=0);
		fclose(fin);
		c = 'y';
		if (argc<2) {
			if (ttyn(0)!='x') {
				printf("Save? ");
				fflush(stdout);
				c = getchar();
			}
		} else
			c = argv[1][1];
		if (c=='y') {
			prepend(".mail", "mbox");
			printf("Saved mail in 'mbox'\n");
		}
		close(creat(".mail", 0666));
	} else
		printf("No mail.\n");
}

void
bulkmail(argc, argv, from)
	char **argv, *from;
{
	long tbuf;
	register int c;

	time(&tbuf);
	fprintf(fout, "From %s %s", from, ctime(&tbuf));
	while ((c = getchar())>=0) {
		putc(c, fout);
	}
	putc('\n', fout);
	fclose(fout);
	while (--argc > 0)
		sendto(*++argv);
	delexit();
}

void
sendto(person)
	char *person;
{
	static int saved;
	register struct passwd *p;
	setpw();
	while (p = getpwent()) {
		if (equal(p->pw_name, person)) {
			if (prepend(lettmp, cat(p->pw_dir, "/.mail"))==0)
				break;
			return;
		}
	}
	printf("Can't send to %s.\n", person);
	fflush(stdout);
	if (ttyn(0)!='x' && saved==0) {
		unlink("dead.letter");
		saved++;
		printf("Letter saved in 'dead.letter'\n");
		prepend(lettmp, "dead.letter");
	}
}

int
prepend(from, to)
	char *from, *to;
{
	register FILE *frd, *fwr;
	register int c;

	fwr = fopen(preptmp, "w");
	frd = fopen(from, "r");
	if(!fwr || !frd)
		goto err;
	while ((c = getc(frd))>=0)
		putc(c,fwr);
	close(frd);
	frd = fopen(to, "r");
	if(frd) {
		while ((c = getc(frd))>=0)
			putc(c,fwr);
		fclose(frd);
	}
	fclose(fwr);
	fwr = fopen(to, "w");
	frd = fopen(preptmp, "r");
	if(!fwr || !frd)
		goto err;
	while ((c = getc(frd))>=0)
		putc(c,fwr);
	fclose(frd);
	fclose(fwr);
	return(1);
err:
	perror(errno);
	return(0);
}

setpw()
{
	if (pwfil == 0) {
		pwfil = fopen("/etc/passwd", "r");
	}
	fseek(pwfil, 0L, 0);
}

struct passwd*
getpwent()
{
	register char *p;
	register int c;
	static struct passwd passwd;
	static char line[100];

	p = line;
	while((c=getc(pwfil)) != '\n') {
		if(c <= 0)
			return(0);
		if(p < line+98)
			*p++ = c;
	}
	*p = 0;
	p = line;
	passwd.pw_name = p;
	p = pwskip(p);
	passwd.pw_passwd = p;
	p = pwskip(p);
	passwd.pw_uid = atoi(p);
	p = pwskip(p);
	passwd.pw_gid = atoi(p);
	p = pwskip(p);
	passwd.pw_gecos = p;
	p = pwskip(p);
	passwd.pw_dir = p;
	p = pwskip(p);
	passwd.pw_shell = p;
	return(&passwd);
}

char *
pwskip(p)
	register char *p;
{
	while(*p != ':') {
		if(*p == 0)
			return(p);
		p++;
	}
	*p++ = 0;
	return(p);
}

void
delexit()
{
	unlink(lettmp);
	unlink(preptmp);
	exit(0);
}

void
maketemp()
{
	int i, pid, d;

	pid = getpid();
	for (i=11; i>=7; --i) {
		d = (pid&07) + '0';
		lettmp[i] = d;
		preptmp[i] = d;
		pid >>= 3;
	}
}

int
equal(s1, s2)
	register char *s1, *s2;
{
	while (*s1++ == *s2)
		if (*s2++ == 0)
			return(1);
	return(0);
}

char*
cat(ap1, ap2)
	char *ap1, *ap2;
{
	register char *p1, *p2;
	static char fn[32];

	p1 = ap1;
	p2 = fn;
	while (*p2++ = *p1++);
	p2--;
	p1 = ap2;
	while (*p2++ = *p1++);
	return(fn);
}

