/*
 *  Print execution profile
 */

#include <stdio.h>
#include <sys/stat.h>

struct nl {
	char     name[8];
	unsigned value;
	long     time;
	long     ncall;
};

struct nl nl[600];

struct fnl {
	char fname[8];
	int  flag;
	int  fvalue;
} nbuf;

struct cnt {
	int  cvalue;
	long cncall;
} cbuf[200];

struct stat statb;

int	buf[17];
int	i;
int	j;
unsigned int	highpc;
unsigned int	lowpc;
unsigned int	ccnt;
unsigned int	pcl;
unsigned int	pch;
int	bufs;
int	nname;
long	time;
long	totime;
long	maxtime;
long	range;
int	scale;
struct nl *np;
struct nl *npe;
int	aflg;
int	vflg;
int	lflg;
int	symoff;
int	symsiz;
int	vf;
int	etext;
int	ncount;

int timcmp();
int valcmp();

main(argc, argv)
	char **argv;
{
	char *namfil;
	int nf, pf, overlap;
	double fnc, ltod(), lastsx;
	struct cnt *cp;

	/* parse arguments */
	argv++;
	namfil = "a.out";
	while (argc>1) {
		if (**argv == '-') {
			if (*++*argv == 'l')
				lflg++;
			if (**argv == 'a')
				aflg = 040;
			if(**argv == 'v')
				vflg++;
		} else
			namfil = *argv;
		argc--;
		argv++;
	}
	/* open executable for symbol names, the 'name file' */
	if ((nf = open(namfil, 0)) < 0) {
		printf("Can't find %s\n", namfil);
		done();
	}
	read(nf, buf, 020);
	if (buf[0] != 0407 && buf[0] != 0410 && buf[0] != 0411) { /* a.out magic */
		printf("Bad format: %s\n", namfil);
		done();
	}
	symsiz = buf[4];
	symoff = buf[1] + buf[2];
	if (buf[7] != 1)
		symoff <<= 1;
	lseek(nf, (long)(symoff+020), 0);

	/* read profile file and get sizes */
	if ((pf = open("mon.out", 0)) < 0) {
		printf("No mon.out\n");
		done();
	}
	fstat(pf, &statb);
	read(pf, &lowpc,  2);
	read(pf, &highpc, 2);
	read(pf, &ncount, 2);
	bufs   = statb.st_size/2 - 3*(ncount+1);

	/* load invocation count list into cbuf */
	read(pf, cbuf, ncount*6);

	/* read in name list */
	npe = nl;
	for (nname = 0; symsiz > 0; symsiz -= 12) {
		read(nf, &nbuf, 12);
		if ((nbuf.flag | aflg) != 042)
			continue;
		npe->value = nbuf.fvalue;
		for (i=0; i<8; i++)
			npe->name[i] = nbuf.fname[i];
		npe++;
		nname++;
	}
	if (nname == 0) {
		printf("No symbols: %s\n", namfil);
		done();
	}
	/* add sentinel with value 0xfffe */
	npe->value = 0xffff;
	npe++;

	/* load invocation counts into name list */
	for (cp = cbuf; cp < &cbuf[ncount]; cp++)
		for (np = nl; np < npe; np++)
			if (cp->cvalue == np->value) {
				np->ncall = cp->cncall;
				break;
			}

	/* process address sample counts */
	qsort(nl, nname, 18, &valcmp);
	range = (highpc-lowpc);
	for (i=0; read(pf, &ccnt, 2) == 2; i++) {
		if (ccnt == 0)
			continue;
		time = ccnt;
		totime += ccnt;
		if(time > maxtime)
			maxtime = time;
		pcl = lowpc + (range*i)/bufs;
		pch = pcl + range/bufs + 1;
		for (j=0; j<nname; j++) {
			if (pch < nl[j].value)
				break;
			if (pcl >= nl[j+1].value)
				continue;
			overlap=(min(pch,nl[j+1].value) - max(pcl,nl[j].value));
			if (overlap<0)
				continue;
			nl[j].time += time * overlap;
		}
	}
	printf("%ld samples, user time %lds\n", totime, totime/60);
	if (totime==0) {
		printf("No time accumulated\n");
		done();
	}
	scale = range/bufs + 1;

	/* print result */
	printf("    name %%time  #call  ms/call\n");
	if (!lflg)
		qsort(nl, nname, 18, &timcmp);
	for (np = nl; np<npe-1; np++) {
		if (!np->time && !np->ncall && !lflg)
			continue;
		time = np->time * 1000;
		time = time / scale;
		time = time / totime;
		printf("%8.8s%4d.%d", np->name, (int)(time/10), (int)(time%10));
		printf("%7ld", np->ncall);
		if (np->ncall) {
			printf(" %8ld\n", np->time*2/np->ncall);
		} else
			printf("\n");
	}
	done();
}

min(a, b)
{
	if (a<b)
		return(a);
	return(b);
}

max(a, b)
{
	if (a>b)
		return(a);
	return(b);
}

valcmp(p1, p2)
struct nl *p1, *p2;
{
	return(p1->value - p2->value);
}

timcmp(p1, p2)
struct nl *p1, *p2;
{
	return(p2->time - p1->time);
}

done()
{

	fflush(stdout);
	exit(0);
}
