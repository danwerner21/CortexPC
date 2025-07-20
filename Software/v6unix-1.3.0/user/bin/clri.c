#define DEBUG
/*
 * clri filsys inumber ...
 */

#include <sys/param.h>
#include <sys/ino.h>
#define ISIZE	(sizeof(struct inode))
#define	BSIZE	512
#define	NI	(BSIZE/ISIZE)
struct	ino
{
	char	junk[ISIZE];
};
struct	ino	buf[NI];
int	status;

#ifdef DEBUG
#include <stdlib.h>
#include <stdio.h>

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

isnumber(s)
char *s;
{
	register c;

	while(c = *s++)
		if(c < '0' || c > '9')
			return(0);
	return(1);
}

main(argc, argv)
char *argv[];
{
	register i, f;
	unsigned n;
	int j, k;
	long off;

	if(argc < 3) {
		printf("usage: clri filsys inumber ...\n");
		exit(4);
	}
	f = open(argv[1], 2);
	if(f < 0) {
		printf("cannot open %s\n", argv[1]);
		exit(4);
	}
	for(i=2; i<argc; i++) {
		if(!isnumber(argv[i])) {
			printf("%s: is not a number\n", argv[i]);
			status = 1;
			continue;
		}
		n = atoi(argv[i]);
		if(n == 0) {
			printf("%s: is zero\n", argv[i]);
			status = 1;
			continue;
		}
		off = ((n-1)/NI + 2) * (long)512;
		lseek(f, off, 0);
		if(read(f, (char *)buf, BSIZE) != BSIZE) {
			printf("%s: read error\n", argv[i]);
			status = 1;
#ifdef DEBUG
		printf ("read block-1: off = %ld\n", off);
		hexdump (stdout, buf, 0, BSIZE);
#endif
		}
	}
	if(status)
		exit(status);
	for(i=2; i<argc; i++) {
		n = atoi(argv[i]);
		printf("clearing %u\n", n);
		off = ((n-1)/NI + 2) * (long)512;
		lseek(f, off, 0);
		read(f, (char *)buf, BSIZE);
#ifdef DEBUG
		printf ("read block-2: off = %ld\n", off);
		hexdump (stdout, buf, 0, BSIZE);
#endif
		j = (n-1)%NI;
		for(k=0; k<ISIZE; k++)
			buf[j].junk[k] = 0;
		lseek(f, off, 0);
		write(f, (char *)buf, BSIZE);
	}
	exit(status);
}
