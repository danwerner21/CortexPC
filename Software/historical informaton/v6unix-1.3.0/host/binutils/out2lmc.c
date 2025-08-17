#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int nflg;
int fi;

struct	filhdr {
	short	fmagic;
	short	tsize;
	short	dsize;
	short	bsize;
	short	ssize;
	short	entry;
	short	unused;
	short	rflag;
} filhdr;

int
readhdr(fd, hdr)
        int fd;
        register struct filhdr *hdr;
{
#ifdef __pdp11__
        if (read(fd, hdr, sizeof(struct exec)) != sizeof(struct exec))
                return 0;
#else
        unsigned char buf [16];

        if (read(fd, buf, 16) != 16)
                return 0;
        hdr->fmagic =	buf[0]  << 8 | buf[1];
        hdr->tsize =	buf[2]  << 8 | buf[3];
        hdr->dsize =	buf[4]  << 8 | buf[5];
        hdr->bsize =	buf[6]  << 8 | buf[7];
        hdr->ssize =	buf[8]  << 8 | buf[9];
        hdr->entry =	buf[10] << 8 | buf[11];
        hdr->unused =	buf[12] << 8 | buf[13];
        hdr->rflag =	buf[14] << 8 | buf[15];
#endif
        return 1;
}

/*
 * Read 16-bit value from file.
 */
unsigned short
getword (fd)
int fd;
{
        unsigned short w,q;
	int d;

        d = read(fd, &q, 2);
#ifndef __ti990__
        w = q << 8;
        w |= (q >> 8 ) & 255;
#endif
        return w;
}

int
main(argc, argv)
	int argc;
	char **argv;
{
	int i, wd, end, le = '\r';

	if (--argc > 0 && *argv[1] == '-') {
		argv++;
		while (*++*argv)
			switch (**argv) {
		
			/* produce \n line endings instead of \r */
			case 'n':
				le = '\n';
				continue;

			default:
				continue;
			}
		argc--;
	}

	if (argc==0)
		fi = open("a.out", 0);
	else
		fi = open(*++argv, 0);
	if(fi < 0) {
		printf("cannot open input\n");
		exit(1);
	}

	readhdr(fi, &filhdr);
	i = filhdr.fmagic;
	if(i!=0407 && i!=0410 && i!=0411) {
		printf("bad format\n");
		exit(1);
	}

	printf("00000OUT2LMC A0000F%c", le);
  
	end = filhdr.tsize + filhdr.dsize;
	for(i=0; i<end; i+=2) {
		wd = getword(fi);
		printf("B%04X", wd);
		if( i%24==22 ) printf("F%c", le);
	}
	if( (i-2)%24!=22 ) printf("F%c", le);

	end = filhdr.bsize;
	for(i=0; i<end; i+=2) {
		printf("B0000");
		if( i%24==22 ) printf("F%c", le);
	}
	if( end && (i-2)%24!=22 ) printf("F%c", le);
	printf(": END %c", le);

	close(fi);
	return(0);
}

  
