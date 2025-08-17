/*
 * acct [ -w wtmp ] [ -d ] [ -p ] [ people ]
 */

#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <utmp.h>
#include <sys/types.h>
#ifdef FTIME
#include <sys/timeb.h>
#endif

#define	TSIZE	33
#define	USIZE	200

static struct  utmp ibuf;

static struct ubuf {
	char	uname[8];
	long	utime;
} ubuf[USIZE];

static struct tbuf {
	struct	ubuf	*userp;
	long	ttime;
} tbuf[TSIZE];

static char	*wtmp;
static int	pflag, byday;
static long	dtime;
static long	midnight;
static long	lastime;
static long	day	= 86400L;
static long	hfday	= 43200L;
static int	pcount;
static char	**pptr;

static int
among(i)
{
	register j, k;
	register char *p;

	if (pcount==0)
		return(1);
	for (j=0; j<pcount; j++) {
		p = pptr[j];
		for (k=0; k<8; k++) {
			if (*p == ubuf[i].uname[k]) {
				if (*p++ == '\0')
					return(1);
			} else
				break;
		}
	}
	return(0);
}

static void
pdate()
{
	long x;
	char *ctime();

	if (byday==0)
		return;
	x = midnight-1;
	printf("%.6s", ctime(&x)+4);
}

static void
print()
{
	int i;
	long ttime, t;
	long lt, rt;

	ttime = 0;
	for (i=0; i<USIZE; i++) {
		if(!among(i))
			continue;
		t = ubuf[i].utime;
		if (t>0)
			ttime += t;
		if (pflag && ubuf[i].utime > 0) {
			lt = (ubuf[i].utime / 3600) * 3600;
			rt = (ubuf[i].utime - lt) / 36;
			printf("\t%-8.8s%6ld.%02ld\n",
			    ubuf[i].uname, ubuf[i].utime / 3600, rt);
		}
	}
	if (ttime > 0) {
		pdate();
		lt = (ttime / 3600) * 3600;
		rt = (ttime - lt) / 36;
		printf("\ttotal%9ld.%02ld\n", ttime / 3600, rt);
	}
}

static void
update(tp, f)
struct tbuf *tp;
{
	int j;
	struct ubuf *up;
	long t, t1;

	if (f)
		t = midnight;
	else
		t = ibuf.ut_time;
	if (tp->userp) {
		t1 = t - tp->ttime;
		if (t1>0 && t1 < (hfday+day))
			tp->userp->utime += t1;
	}
	tp->ttime = t;
	if (f)
		return;
	if (ibuf.ut_name[0]=='\0') {
		tp->userp = 0;
		return;
	}
	for (up=ubuf; up < &ubuf[USIZE]; up++) {
		if (up->uname[0] == '\0')
			break;
		for (j=0; j<8 && up->uname[j]==ibuf.ut_name[j]; j++);
		if (j>=8)
			break;
	}
	for (j=0; j<8; j++)
		up->uname[j] = ibuf.ut_name[j];
	tp->userp = up;
}

static void
upall(f)
{
	register struct tbuf *tp;

	for (tp=tbuf; tp < &tbuf[TSIZE]; tp++)
		update(tp, f);
}

static void
newday()
{
	long ttime;
#ifdef FTIME
	struct timeb tb;
#endif
	struct tm *localtime();

	time(&ttime);
#ifdef FTIME
	if (midnight == 0) {
		ftime(&tb);
		midnight = 60*(long)tb.timezone;
		if (localtime(&ttime)->tm_isdst)
			midnight -= 3600;
	}
#endif
	while (midnight <= ibuf.ut_time)
		midnight += day;
}

static void
loop()
{
	register i;
	register struct tbuf *tp;
	register struct ubuf *up;

	if(ibuf.ut_line[0] == '|') {
		dtime = ibuf.ut_time;
		return;
	}
	if(ibuf.ut_line[0] == '}') {
		if(dtime == 0)
			return;
		for(tp = tbuf; tp < &tbuf[TSIZE]; tp++)
			tp->ttime += ibuf.ut_time-dtime;
		dtime = 0;
		return;
	}
	if (lastime>ibuf.ut_time || lastime+(hfday+day)<ibuf.ut_time)
		midnight = 0;
	if (midnight==0)
		newday();
	lastime = ibuf.ut_time;
	if (byday && ibuf.ut_time > midnight) {
		upall(1);
		print();
		newday();
		for (up=ubuf; up < &ubuf[USIZE]; up++)
			up->utime = 0;
	}
	if (ibuf.ut_line[0] == '~') {
		ibuf.ut_name[0] = '\0';
		upall(0);
		return;
	}
	if (ibuf.ut_line[0]=='t')
		i = (ibuf.ut_line[1]-'0');
	else
		i = TSIZE-1;
	if (i<0 || i>=TSIZE)
		i = TSIZE-1;
	tp = &tbuf[i];
	update(tp, 0);
}

main(argc, argv) 
char **argv;
{
	int c, fl;
	register i;
	FILE *wf;

	wtmp = "/usr/adm/wtmp";
	while (--argc > 0 && **++argv == '-')
	switch(*++*argv) {
	case 'd':
		byday++;
		continue;

	case 'w':
		if (--argc>0)
			wtmp = *++argv;
		continue;

	case 'p':
		pflag++;
		continue;
	}
	pcount = argc;
	pptr = argv;
	if ((wf = fopen(wtmp, "r")) == NULL) {
		printf("No %s\n", wtmp);
		exit(1);
	}
	for(;;) {
		if (fread((char *)&ibuf, sizeof(ibuf), 1, wf) != 1)
			break;
		fl = 0;
		for (i=0; i<8; i++) {
			c = ibuf.ut_name[i];
			if(isdigit(c) || isalpha(c)) {
				if (fl)
					goto skip;
				continue;
			}
			if (c==' ' || c=='\0') {
				fl++;
				ibuf.ut_name[i] = '\0';
			} else
				goto skip;
		}
		loop();
    skip:;
	}
	ibuf.ut_name[0] = '\0';
	ibuf.ut_line[0] = '~';
	time(&ibuf.ut_time);
	loop();
	print();
	exit(0);
}
