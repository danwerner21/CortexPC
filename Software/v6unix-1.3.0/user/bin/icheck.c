/*#define DEBUG*/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/ino.h>
#include <sys/filsys.h>
#include <sys/fblk.h>

char	*dargv[] =
{
	"/dev/dsk0",
	"/dev/dsk1",
	"/dev/dsk2",
	"/dev/dsk3",
	0
};
#define	NINODE	16*16
#define	NB	10

struct	inode	inode[NINODE];
struct	filsys	sblock;

int	sflg;
int	fi;
int	nifiles;
int	nfile;
int	nspcl;
int	nlarg;
int	nvlarg;
int	nindir;
int	nvindir;
int	ndir;
int	nused;
int	nfree;
int	ino;
int	ndup;
int	blist[10] = { -1};
int	nerror;
int	usedarg;
int	bmap[4096];

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
	register *ip;
	long bn = bno;

	lseek(fi, bn<<BSHIFT, 0);
	if (read(fi, buf, cnt) != cnt) {
		if (bno == 1L && usedarg) return (-1);
		printf("read error %d\n", bno);
		if (sflg) {
			printf("No update\n");
			sflg = 0;
		}
		for (ip = (int *)buf; ip < &buf[256];)
			*ip++ = 0;
	}
#ifdef DEBUG
	printf ("bread: bno = %d, cnt = %d\n", bno, cnt);
	hexdump (stdout, buf, 0, cnt);
#endif
	return (0);
}

alloc()
{
	register i;
	register b;
	struct fblk buf;

	i = --sblock.s_nfree;
#ifdef DEBUG
	printf ("alloc: i = %d\n", i);
#endif
	if (i<0 || i>=NICFREE) {
		printf("bad freeblock: %d\n", i);
		return(0);
	}
	b = sblock.s_free[i];
#ifdef DEBUG
	printf ("   b = %d\n", b);
#endif
	if (b == 0)
		return(0);
#ifdef DEBUG
	printf ("   s_isize = %d, s_fsize = %d, s_nfree = %d\n", 
		sblock.s_isize, sblock.s_fsize, sblock.s_nfree);
#endif
	if (sblock.s_nfree <= 0) {
		bread(b, (char *)&buf, sizeof(buf));
		sblock.s_nfree = buf.df_nfree;
		for(i=0; i<NICFREE; i++)
			sblock.s_free[i] = buf.df_free[i];
	}
	return(b);
}

chk(ab, s)
char *ab;
{
	register char *b;
	register n, m;

	b = ab;
	if (ino)
		nused++;
	if (b<sblock.s_isize+2 || b>=sblock.s_fsize) {
		printf("%d bad; inode=%d, class=%s\n", b, ino, s);
		return(1);
	}
	m = 1 << (((int)b)&017);
	n = (((int)b)>>4) & 07777;
	if (bmap[n]&m) {
		printf("%d dup; inode=%d, class=%s\n", b, ino, s);
		ndup++;
	}
	bmap[n] |= m;
	for (n=0; blist[n] != -1; n++)
		if (b == blist[n])
			printf("%d arg; inode=%d, class=%s\n", b, ino, s);
	return(0);
}

pass1(aip)
struct inode *aip;
{
	int buf[256], vbuf[256];
	register i, j;
	struct inode *ip;

	ip = aip;
	if ((ip->i_mode&IALLOC) == 0)
		return;
	if ((ip->i_mode&IFCHR&IFBLK) != 0) {
		nspcl++;
		return;
	}
	if ((ip->i_mode&IFMT) == IFDIR)
		ndir++;
	else
		nfile++;
	if ((ip->i_mode&ILARG) != 0) {
		nlarg++;
		for(i=0; i<7; i++)
		if (ip->i_addr[i] != 0) {
			nindir++;
			if (chk(ip->i_addr[i], "indirect"))
				continue;
			bread(ip->i_addr[i], (char *)buf, 512);
			for(j=0; j<256; j++)
			if (buf[j] != 0)
				chk(buf[j], "data (large)");
		}
		if (ip->i_addr[7]) {
			nvlarg++;
			if (chk(ip->i_addr[7], "indirect"))
				return;
			bread(ip->i_addr[7], (char *)buf, 512);
			for(i=0; i<256; i++)
			if (buf[i] != 0) {
				nvindir++;
				if (chk(buf[i], "2nd indirect"))
					continue;
				bread(buf[i], (char *)vbuf, 512);
				for(j=0; j<256; j++)
				if (vbuf[j])
					chk(vbuf[j], "data (very large)");
			}
		}
		return;
	}
	for(i=0; i<8; i++) {
		if (ip->i_addr[i] != 0)
			chk(ip->i_addr[i], "data (small)");
	}
}

bwrite(bno, buf)
char *buf;
{
	long bn = bno;
	lseek(fi, bn<<BSHIFT, 0);
	if (write(fi, buf, 512) != 512)
		printf("write error %d\n", bno);
}

ifree(in)
{
	register i;
	struct fblk buf;

	if (sblock.s_nfree >= 100) {
		buf.df_nfree = sblock.s_nfree;
		for(i=0; i<NICFREE; i++)
			buf.df_free[i] = sblock.s_free[i];
		sblock.s_nfree = 0;
		bwrite(in, (char *)&buf);
	}
	sblock.s_free[sblock.s_nfree++] = in;
}

makefree()
{
	register i;

	sblock.s_nfree = 0;
	sblock.s_ninode = 0;
	sblock.s_flock = 0;
	sblock.s_ilock = 0;
	sblock.s_fmod = 0;
	ifree(0);
	for(i=sblock.s_fsize-1; i>=sblock.s_isize+2; i--) {
		if ((bmap[(i>>4)&07777] & (1<<(i&017)))==0)
			ifree(i);
	}
	bwrite(1L, (char *)&sblock);
	close(fi);
	sync();
	return;
}

check(file)
char *file;
{
	register *ip, i, j;

#ifdef DEBUG
	printf ("check: file = %s\n", file);
#endif
	fi = open(file, sflg?2:0);
	if (fi < 0) {
		printf("cannot open %s\n", file);
		nerror |= 04;
		return;
	}
	nfile = 0;
	nspcl = 0;
	nlarg = 0;
	nvlarg = 0;
	nindir = 0;
	nvindir = 0;
	ndir = 0;
	nused = 0;
	nfree = 0;
	ndup = 0;
	for (i = 0; i < 4096; i++)
		bmap[i] = 0;
	sync();
	if (bread(1, (char *)&sblock, 512) < 0) return;
	printf("%s:\n", file);
	nifiles = sblock.s_isize*16;
	for(i=0; ino < nifiles; i += NINODE/16) {
		bread(i+2, (char *)inode, sizeof inode);
		for(j=0; j<NINODE && ino<nifiles; j++) {
			ino++;
			pass1(&inode[j]);
		}
	}
	ino = 0;
	sync();
	bread(1, (char *)&sblock, 512);
	if (sflg) {
		makefree();
		return;
	}
	while(i = alloc()) {
		if (chk(i, "free"))
			break;
		nfree++;
	}
	if (ndup) {
		printf("%d dups in free\n", ndup);
		nerror |= 02;
	}
	j = 0;
	for (ip = bmap; ip < &bmap[4096];) {
		i = *ip++;
		while (i) {
			if (i<0)
				j--;
			i <<= 1;
		}
	}
	j += sblock.s_fsize - sblock.s_isize - 2;
	if (j)
		printf("missing%5d\n", j);
	printf("spcl  %6u\n", nspcl);
	printf("files %6u\n", nfile);
	printf("large %6u\n", nlarg);
	if (nvlarg)
		printf("huge  %6u\n", nvlarg);
	printf("direc %6u\n", ndir);
	printf("indir %6u\n", nindir);
	if (nvindir)
		printf("indir2%6u\n", nvindir);
	printf("used  %6u\n", nused);
	printf("free  %6u\n", nfree);
	close(fi);
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

main(argc, argv)
char **argv;
{
	register char **p;
	register int n, *lp;

	usedarg = 0;
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

		case 'b':
			lp = blist;
			while (lp < &blist[NB-1] && (n = number(argv[1]))) {
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
