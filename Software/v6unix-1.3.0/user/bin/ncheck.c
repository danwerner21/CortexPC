/*#define DEBUG*/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/ino.h>
#include <sys/filsys.h>
#include <sys/dir.h>

char	*dargv[] =
{
	"/dev/dsk0",
	"/dev/dsk1",
	"/dev/dsk2",
	"/dev/dsk3",
	0
};

#define NINODE	16*16

struct	filsys	sblock;
struct	inode	inode[NINODE];

int	sflg;
int	aflg;
#define	NI	20
#define	NDIRS	787

int	ilist[NI] = { -1};
int	fi;
struct	htab {
	int	hino;
	int	hpino;
	char	hname[14];
} htab[NDIRS];

int	nhent	= 10;
char	*lasts;
int	ino;
int	nerror;
int	nffil;
int	fout;
int	nfiles;
int	usedarg;

#ifdef DEBUG
#include <stdlib.h>

hexdump (file, ptr, offset, size)
	FILE *file; char *ptr;
{
   int jjj;
   int iii;
   char *tp;
   char *cp;

   for (iii = 0, tp = (char *)ptr, cp = (char *)ptr; iii < size; )
   {
      fprintf (file, "%04X   ", iii + offset);
      for (jjj = 0; jjj < 8; jjj++)
      {
	 if (cp < ((char *)ptr + size))
	 {
	    fprintf (file, "%2.2X", *cp++ & 0xFF);
	    if (cp < ((char *)ptr + size))
	    {
	       fprintf (file, "%2.2X ", *cp++ & 0xFF);
	    }
	    else
	    {
	       fprintf (file, "   ");
	    }
	 }
	 else
	 {
	    fprintf (file, "     ");
	 }
	 iii += 2;
      }

      fprintf (file, "   ");
      for (jjj = 0; jjj < 8; jjj++)
      {
	 if (tp < ((char *)ptr + size))
	 {
	    if (*tp >= 0x20 && *tp < 0x7F)
	       fprintf (file, "%c", *tp);
	    else
	       fprintf (file, ".");
	    tp++;
	    if (tp < ((char *)ptr + size))
	    {
	       if (*tp >= 0x20 && *tp < 0x7F)
		  fprintf (file, "%c ", *tp);
	       else
		  fprintf (file, ". ");
	       tp++;
	    }
	    else
	    {
	       fprintf (file, "  ");
	    }
	 }
	 else
	 {
	    fprintf (file, "   ");
	 }
      }
      fprintf (file, "\n");
   }
}
#endif

printf2(s, a1, a2)
{
	extern fout;

	fflush(stdout);
	fout = 2;
	printf(s, a1, a2);
	fout = nffil;
	fflush(stdout);
}

bread(bno, buf, cnt)
char *buf;
{
#ifdef DEBUGBREAD
	int i, j;
#endif
	long bn = bno;
	lseek(fi, bn<<BSHIFT, 0);
	if (read(fi, buf, cnt) != cnt) {
		if (bno == 1 && usedarg) return (-1);
		printf2("read error %d\n", bno);
		exit();
	}
#ifdef DEBUGBREAD
	for (i = j = 0; i < cnt; i++)
		if (buf[i]) {
			j = 1;
			break;
		}
	printf ("bread: bno = %d, cnt = %d\n", bno, cnt);
	if (j)
	   hexdump (stdout, buf, 0, cnt);
#endif
	return (0);
}

dotname(adp)
struct direct *adp;
{
	register struct direct *dp;

	dp = adp;
	if (dp->d_name[0]=='.')
		if (dp->d_name[1]==0 || dp->d_name[1]=='.' && dp->d_name[2]==0)
			return(1);
	return(0);
}

struct htab *
lookup(i, ef)
{
	register struct htab *hp;

	for (hp = &htab[i%NDIRS]; hp->hino;) {
		if (hp->hino==i)
			return(hp);
		if (++hp >= &htab[NDIRS])
			hp = htab;
	}
	if (ef==0)
		return(0);
	if (++nhent >= NDIRS) {
		printf2("Out of core-- increase NDIRS\n");
		fflush(stdout);
		exit(1);
	}
	hp->hino = i;
	return(hp);
}

pname(i, lev)
{
	register struct htab *hp;

	if (i==1)
		return;
	if ((hp = lookup(i, 0)) == 0) {
		printf("???");
		return;
	}
	if (lev > 10) {
		printf("...");
		return;
	}
	pname(hp->hpino, ++lev);
	printf("/%.14s", hp->hname);
}

struct direct *
dread(aip, aoff)
struct inode *aip;
{
	register b, off;
	register struct inode *ip;
	static ibuf[256];
	static char buf[512];

	off = aoff;
	ip = aip;
	if ((off&0777)==0) {
		if (off==0177000) {
			printf2("Monstrous directory %l\n", ino);
			return(0);
		}
		if ((ip->i_mode&ILARG)==0) {
			if (off>=010000 || (b = ip->i_addr[off>>9])==0)
				return(0);
			bread(b, buf, 512);
		} else {
			if (off==0) {
				if (ip->i_addr[0]==0)
					return(0);
				bread(ip->i_addr[0], ibuf, 512);
			}
			if ((b = ibuf[(off>>9)&0177])==0)
				return(0);
			bread(b, buf, 512);
		}
	}
	return((struct direct *)&buf[off&0777]);
}

number(as)
char *as;
{
	register n, c;
	register char *s;

	s = as;
	n = 0;
	while ((c = *s++) >= '0' && c <= '9') {
		n = n*10+c-'0';
	}
	return(n);
}

pass1(ip)
struct inode *ip;
{
	if ((ip->i_mode&IALLOC)==0 || (ip->i_mode&IFMT)!=IFDIR)
		return;
	lookup(ino, 1);
}

pass2(ip)
struct inode *ip;
{
	register doff;
	register struct htab *hp;
	register struct direct *dp;
	int i;

	if ((ip->i_mode&IALLOC)==0 || (ip->i_mode&IFMT)!=IFDIR)
		return;
	doff = 0;
	while (dp = dread(ip, doff)) {
		doff += 16;
		if (dp->d_ino==0)
			continue;
		if ((hp = lookup(dp->d_ino, 0)) == 0)
			continue;
		if (dotname(dp))
			continue;
		hp->hpino = ino;
		for (i=0; i<14; i++)
			hp->hname[i] = dp->d_name[i];
	}
}

pass3(ip)
struct inode *ip;
{
	register doff;
	register struct direct *dp;
	register int *ilp;

	if ((ip->i_mode&IALLOC)==0 || (ip->i_mode&IFMT)!=IFDIR)
		return;
	doff = 0;
	while (dp = dread(ip, doff)) {
		doff += 16;
		if (dp->d_ino==0)
			continue;
		if (aflg==0 && dotname(dp))
			continue;
		for (ilp=ilist; *ilp >= 0; ilp++)
			if (*ilp == dp->d_ino)
				break;
		if (ilp > ilist && *ilp!=dp->d_ino)
			continue;
		printf("%d	", dp->d_ino);
		pname(ino, 0);
		printf("/%.14s\n", dp->d_name);
	}
}

int	(*pass[])()	= { pass1, pass2, pass3 };

check(file)
char *file;
{
	register i, j, pno;

#ifdef DEBUG
	printf ("check: fs = %s\n", file);
#endif
	fi = open(file, 0);
	if (fi < 0) {
		printf2("cannot open %s\n", file);
		return;
	}
	sync();
	if (bread(1, (char *)&sblock, 512) < 0) return;
	printf2("%s:\n", file);
	nfiles = sblock.s_isize*16;
	for (i=0; i<NDIRS; i++)
		htab[i].hino = 0;
	fout = nffil;
	fflush(stdout);
	for (pno=0; pno<3; pno++) {
		ino = 0;
#ifdef DEBUG
		printf ("calling pass%d():\n", pno+1);
#endif
		for (i=0; ino<nfiles; i += NINODE/16) {
			bread(i+2, (char *)inode, sizeof inode);
			for (j=0; j<NINODE && ino<nfiles; j++) {
				ino++;
				(*pass[pno])(&inode[j]);
			}
		}
	}
	fflush(stdout);
	fout = 1;
}

main(argc, argv)
char **argv;
{
	register char **p;
	register int n, *lp;

	usedarg = 0;
	nffil = dup(1);
	if (argc == 1) {
		usedarg = 1;
		for (p = dargv; *p;)
			check(*p++);
		return(nerror);
	}
	while (--argc) {
		argv++;
		if (**argv=='-') switch ((*argv)[1]) {
		case 's':
			sflg++;
			continue;

		case 'a':
			aflg++;
			continue;

		case 'i':
			lp = ilist;
			while (lp < &ilist[NI-1] && (n = number(argv[1]))) {
				*lp++ = n;
				argv++;
				argc--;
			}
			*lp++ = -1;
			continue;

		default:
			printf2("Bad flag\n");
		}
		check(*argv);
	}
	return(nerror);
}
