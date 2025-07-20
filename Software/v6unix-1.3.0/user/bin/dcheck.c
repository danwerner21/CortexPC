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
#define	NI	20

struct	inode	inode[NINODE];
struct	filsys	sblock;

int	sflg;
int	headpr;

int	ilist[NI]  = { -1};
int	fi;
char	*ecount;
char	*lasts;
int	ino;
int	nerror;
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

bread(bno, buf, cnt)
char *buf;
{
	long bn = bno;
	lseek(fi, bn<<BSHIFT, 0);
	if(read(fi, buf, cnt) != cnt) {
		if (bno == 1 && usedarg) return (-1);
		printf("read error %d\n", bno);
		exit();
	}
#ifdef DEBUG
	printf ("bread: bno = %d, cnt = %d\n", bno, cnt);
	hexdump (stdout, buf, 0, cnt);
#endif
	return (0);
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
			printf("Monstrous directory %d\n", ino);
			return(0);
		}
		if ((ip->i_mode&ILARG)==0) {
			if (off>=010000 || (b = ip->i_addr[off>>9])==0)
				return(0);
			bread(b, (char *)buf, 512);
		} else {
			if (off==0) {
				if (ip->i_addr[0]==0)
					return(0);
				bread(ip->i_addr[0], (char *)ibuf, 512);
			}
			if ((b = ibuf[(off>>9)&0177])==0)
				return(0);
			bread(b, (char *)buf, 512);
		}
	}
	return((struct direct *)&buf[off&0777]);
}

pass1(aip)
struct inode *aip;
{
	register doff;
	register struct inode *ip;
	register struct direct *dp;
	int i;

	ip = aip;
	if((ip->i_mode&IALLOC) == 0)
		return;
	if((ip->i_mode&IFMT) != IFDIR)
		return;
	doff = 0;
	while (dp = (struct direct *)dread(ip, doff)) {
		doff += 16;
		if (dp->d_ino==0)
			continue;
		for (i=0; ilist[i] != -1; i++)
			if (ilist[i]==dp->d_ino)
				printf("%5d arg; %d/%.14s\n",
					dp->d_ino, ino, dp->d_name);
		ecount[dp->d_ino]++;
	}
}

pass2(aip)
struct inode *aip;
{
	register struct inode *ip;
	register i;

	ip = aip;
	i = ino;
	if ((ip->i_mode&IALLOC)==0 && ecount[i]==0)
		return;
	if (ip->i_nlink==ecount[i] && ip->i_nlink!=0)
		return;
	if (headpr==0) {
		printf("entries	link cnt\n");
		headpr++;
	}
	printf("%d	%d	%d\n", ino,
	    ecount[i]&0377, ip->i_nlink&0377);
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

check(file)
char *file;
{
	register i, j;
	fi = open(file, 0);
	if(fi < 0) {
		printf("cannot open %s\n", file);
		return;
	}
	headpr = 0;
	sync();
	if (bread(1, (char *)&sblock, 512) < 0) return;
	printf("%s:\n", file);
	nfiles = sblock.s_isize*16;
	if (lasts < nfiles) {
		if ((sbrk(nfiles - lasts)) == -1) {
			printf("Not enough core\n");
			exit(04);
		}
		lasts = (char *)nfiles;
	}
	for (i=0; i<nfiles; i++)
		ecount[i] = 0;
	for(i=0; ino<nfiles; i += NINODE/16) {
		bread(i+2, (char *)inode, sizeof inode);
		for(j=0; j<NINODE && ino<nfiles; j++) {
			ino++;
			pass1(&inode[j]);
		}
	}
	ino = 0;
	for (i=0; ino<nfiles; i += NINODE/16) {
		bread(i+2, (char *)inode, sizeof inode);
		for (j=0; j<NINODE && ino<nfiles; j++) {
			ino++;
			pass2(&inode[j]);
		}
	}
}

main(argc, argv)
char **argv;
{
	register char **p;
	register int n, *lp;

	usedarg = 0;
	ecount = sbrk(0);
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
			printf("Bad flag\n");
		}
		check(*argv);
	}
	return(nerror);
}
