/*
 * Convert an a.out file to a Powertran Cortex cassette file.
 * 
 * Closely based on the 'makecas' utility by 'tms9995' but
 * rewritten to match the ancient coding style of old Unix.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/* a.out header */
struct outhdr {
	unsigned short	fmagic;
	unsigned short	tsize;
	unsigned short	dsize;
	unsigned short	bsize;
	unsigned short	ssize;
	unsigned short	entry;
	unsigned short	unused;
	unsigned short	rflag;
} outhdr;

/* .cas header */
struct {
	unsigned char   autorun[2];
	unsigned char   idt[8];
	unsigned char   zero[2];
	unsigned char   loadaddr[2];
	unsigned char   startaddr[2];
	unsigned char   length[2];
	unsigned char   chksumMSB;
	unsigned char   chksumLSB;

} cashdr;

int
usage(void)
{
	printf ("usage: out2cas [-x] -o outfile.cas infile\n");
	printf ("    -x: make cassette image auto-run\n");
	exit(1);
}

/* Read an a.out header, which is in big-endian byte order */
int
readhdr(fd, hdr)
        int fd;
        register struct outhdr *hdr;
{
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
        return 1;
}

char ETX = '\003';

int
main(argc, argv)
	int argc;
	char* argv[];
{
	char 	*bp;
	int 	autorun  = 0;
	char 	*infile  = NULL;
	char 	*outfile = NULL;
	int	i, ifd, ofd;

	unsigned char  progdata[65536];
	unsigned short progsize, initsize, startadr;

	unsigned short chksum, csrev;

	if(argc < 4) {
		usage();
		return 1;
	}

	for (i = 1; i < argc; i++)
	{
		bp = argv[i];

		if (*bp == '-') {
			for (bp++; *bp; bp++) switch (*bp)
			{
			case 'o':
				if (++i >= argc) {
					printf("missing outfile\n");
					usage();
					return 1;
				}
				outfile = argv[i];
				break;

			case 'x':
				autorun = 1;
				break;

			default:
				printf("unknown option\n");
				usage();
			}
		}
		else {
			if (infile) {
				printf("more than one input file\n");
				usage();
			}
			infile = bp;
		}
	}	if(!infile) {
		printf("no input file specified\n");
		usage();
	}

	if(!outfile) {
		printf("no output file specified\n");
		usage();
	}

	/* Read image from file, initializing bss area to zero */
	if ((ifd = open(infile,0)) < 0) {
ierr:
		printf("error reading input file\n");
		exit(1);
	}

	if (!readhdr(ifd, &outhdr)) goto ierr;

	memset(progdata, 0, 65536);
	initsize = outhdr.tsize + outhdr.dsize;
	progsize = initsize + outhdr.bsize;
	startadr = outhdr.entry;

	if (read(ifd, progdata, initsize) != initsize) goto ierr;

	close(ifd);

	/* Build the .cas header info */
	cashdr.autorun[0] = cashdr.autorun[1] = autorun ? 0xa5 : 0x5a;

	for(i=0; i < 8; i++)
		cashdr.idt[i] = 0;

	cashdr.zero[0] = cashdr.zero[1] = 0;

	cashdr.loadaddr[0] = (unsigned char) (startadr >> 8);
	cashdr.loadaddr[1] = (unsigned char) (startadr & 0xff);

	cashdr.startaddr[0] = (unsigned char) (startadr >> 8);
	cashdr.startaddr[1] = (unsigned char) (startadr & 0xff);

	cashdr.length[0] = (unsigned char) (progsize >> 8);
	cashdr.length[1] = (unsigned char) (progsize & 0xff);

	/* Calc and set header checksum */
	chksum = 0;
	bp = cashdr.autorun;
	for(i=0; i < 18; i+=2)
		chksum += (bp[i] << 8) + bp[i+1];
	cashdr.chksumMSB = (chksum & 0xff)  >> 8;
	cashdr.chksumLSB = chksum & 0xFF;

	/* Calc and set program checksum */
	chksum = 0;
	for(i=0; i < progsize; i++)
		chksum += progdata[i];
	csrev = (chksum << 8) + (chksum >>8);

	/* Write cassette file */
	if ((ofd = creat(outfile,0644)) < 1) {
oerr:
		printf("error writing output file\n");
		return 1;
	}
	if (write(ofd, &cashdr, 20) != 20) goto oerr;
	if (write(ofd, &progdata, progsize) != progsize) goto oerr;
	if (write(ofd, &ETX, 1) != 1) goto oerr;
	if (write(ofd, &csrev, 2) != 2) goto oerr;
	close(ofd);

	return 0;
}

