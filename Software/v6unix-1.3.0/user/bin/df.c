#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/filsys.h>
#include <sys/fblk.h>

#define	NMOUNT	16
#define	NAMSIZ	32

struct mtab {
	char	file[NAMSIZ];
	char	spec[NAMSIZ];
};


static long blkno = 1;
static char *dargv[] = {
	0,
	"/dev/dsk0",
	"/dev/dsk1",
	"/dev/dsk2",
	"/dev/dsk3",
	0
};


static struct filsys sblock;
static struct mtab mtable[NMOUNT];

static int fi;
static int usedarg;
static int first;
static int mtablen;

static struct fblk obuf;
static long ob;
static int bcnt;

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
	    fprintf (file, "%02X", *cp++ & 0xFF);
	    if (cp < ((char *)ptr + size))
	    {
	       fprintf (file, "%02X ", *cp++ & 0xFF);
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

int
bread(file, bno, buf, cnt)
char *file;
long bno;
char *buf;
{
	int n;

#ifdef DEBUGDISK
	printf ("bread: bno = %ld\n", bno);
#endif
	lseek(fi, bno<<BSHIFT, 0);
	if((n=read(fi, buf, cnt)) != cnt) {
		if (bno == 1L && usedarg)
			return -1;
		fprintf (stderr,"Cannot read %s: %s\n",
		   	   file, sys_errlist[errno]);
		exit(0);
	}
	bcnt++;
#ifdef DEBUGDISK
	printf ("read block, bcnt = %d, bno = %ld, cnt = %d\n", bcnt, bno, cnt);
	hexdump (stdout, buf, 0, cnt);
#endif
	return 0;
}

long
alloc(file)
char *file;
{
	int i;
	long b;
	struct fblk buf;

	i = --sblock.s_nfree;
#ifdef DEBUG
	printf ("alloc: i = %d\n", i);
#endif
	if(i<0 || i>=NICFREE) {
		fprintf(stderr, "Bad free count: s_nfree = %d, blkno = %ld\n",
			i, blkno);
		return(0L);
	}
	b = sblock.s_free[i];
#ifdef DEBUG
	printf ("   b = %ld\n", b);
#endif
	if(b == 0)
		return(0L);
#ifdef DEBUG
	printf ("   s_isize = %u, s_fsize = %u\n", 
		sblock.s_isize, sblock.s_fsize);
#endif
	if(b<sblock.s_isize || b>=sblock.s_fsize) {
		fprintf(stderr, "Bad free block: b = %ld\n", b);
		return(0L);
	}
	if(sblock.s_nfree <= 0) {
		bread(file, b, (char *)&buf, sizeof(buf));
		blkno = b;
		sblock.s_nfree = buf.df_nfree;
		for(i=0; i<NICFREE; i++)
			sblock.s_free[i] = buf.df_free[i];
		if (sblock.s_nfree < 0 || sblock.s_nfree > NICFREE) {
		   fprintf(stderr,
			 "Bad free count: bcnt = %d, nfree = %d, b = %ld\n",
			   bcnt, sblock.s_nfree, blkno);
#ifdef DEBUG
		   fprintf (stderr, "buf:\n");
		   hexdump (stderr, (char *)&buf, 0, sizeof(buf));
		   fprintf (stderr, "obuf: b = %ld\n", ob);
		   hexdump (stderr, (char *)&obuf, 0, sizeof(obuf));
#endif
		   exit (1);
		}
		memcpy ((char *)&obuf, (char *)&buf, sizeof(obuf));
		ob = b;
	}
	return(b);
}

dfree(file)
char *file;
{
	char *mp;
	long i, u, f, p;
	int j;

#ifdef DEBUG
	printf ("dfree: file = %s\n", file);
#endif
	fi = open(file, 0);
	if(fi < 0) {
		if (!usedarg)
		   fprintf(stderr,"Cannot open %s: %s\n",
		   	   file, sys_errlist[errno]);
		return;
	}
	sync();
	if (bread(file, 1L, (char *)&sblock, sizeof(sblock)) < 0) {
		close(fi);
		return;
	}
#ifdef DEBUG
	printf ("   isize = %u, fsize = %u, nfree = %u, ninode = %u\n",
	sblock.s_isize, sblock.s_fsize, sblock.s_nfree, sblock.s_ninode);
#endif
	i = 0;
	while(alloc(file))
		i++;
	if (first) {
	     first = 0;
	     printf (
		"Filesystem    512B-blocks       Used  Available Use% Mount\n");
	}
	mp = "";
	for (j = 0; j < NMOUNT; j++) {
	   if (!strcmp (mtable[j].spec, &file[5])) {
	      mp = mtable[j].file;
	      break;
	   }
	}

	f = (long)sblock.s_fsize;
	p = u = f - i;
	p = (p * 100) / f;
	printf("%-14.14s %10ld %10ld %10ld %3ld%% %s\n",
		file, f, u, i, p, mp);
	close(fi);
}

main(argc, argv)
char **argv;
{
	int i, j, mf;
	char dev[64];

	memset ((char *)&obuf, 0, sizeof(obuf));
	mtablen = 0;
	if ((mf = open("/etc/mtab", 0)) > 0) {
		mtablen = read (mf, mtable, sizeof(mtable));
		close (mf);
		mtablen = mtablen / sizeof (struct mtab);
	}

	usedarg = 0;
	first = 1;
	if(argc <= 1) {
		if (mtablen) {
			for (i = 0; i < mtablen; i++) {
				sprintf (dev, "/dev/%s", mtable[i].spec);
				dfree(dev);
			}
			exit (0);
		} else {
			/* Ignore open/read errors, non-existant dev */
			usedarg = 1;
			for(argc = 1; dargv[argc]; argc++);
			argv = dargv;
		}
	}

	for(i=1; i<argc; i++) {
		strcpy (dev, argv[i]);
		for (j = 0; j < mtablen; j++) {
			if (!strcmp (argv[i], mtable[j].file)) {
				sprintf (dev, "/dev/%s", mtable[j].spec);
				break;
			}
		}
		dfree(dev);
	}
}
