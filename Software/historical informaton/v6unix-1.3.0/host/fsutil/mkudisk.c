/***********************************************************************
*
* MKUDISK - Make a TI990 TILINE Unix disk image file.
*
* Changes:
*      12/17/03   DGP   Original
*      07/15/04   DGP   Corrected DS80 info.
*      07/14/15   DGP   Modified to support lsx/unix disks.
*      01/17/16   DGP   Modified to support 512 byte sector disks.
*      04/07/16   DGP   Added DS10 overhead support.
*
***********************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>

#include "support.h"
#include "simdsk.h"

uint8 memory[SYSMEMSIZE];

/***********************************************************************
* write_overhead
***********************************************************************/

static int
write_overhead (FILE *dfd, int overhead)
{
   int i;

   if (overhead)
   {
      for (i = 0; i < overhead; i++)
	 fputc ('\0', dfd);
   }
   return (0);
}

/***********************************************************************
* write_boot
***********************************************************************/

static int
write_boot (FILE *dfd, FILE *bfd, char *disklabel, char *imagename,
	    int sectrk, int secsize, int overhead)
{
   int pgmlen;
   int i;
   int loadaddr = 0xa0;
   uint8 sector[MAXSECSIZE];

   memset (sector, 0, sizeof(sector));
   memset (memory, 0, sizeof(memory));

   /*
   ** Load the track 1 loader.
   */

   pgmlen = 0;
   if (bfd)
   {
      if ((pgmlen = binloader (bfd, loadaddr+8)) < 0)
         return (-1);
   }

   /*
   ** Initialize the Volume sector.
   */

   sprintf ((char *)&sector[0], "%-8.8s", disklabel);

   sprintf ((char *)&sector[0x2E], "%-8.8s", imagename);
   sprintf ((char *)&sector[0x36], "%-8.8s", "");

   sprintf ((char *)&sector[0x4A], "%-8.8s", "");
   sprintf ((char *)&sector[0x52], "%-8.8s", "");

   sprintf ((char *)&sector[0x5C], "%-8.8s", "");
   sprintf ((char *)&sector[0x64], "%-8.8s", "");

   sprintf ((char *)&sector[0x6E], "%-8.8s", "");
   sprintf ((char *)&sector[0x74], "%-8.8s", "");

   sprintf ((char *)&sector[0x90], "%-8.8s", "");
   sprintf ((char *)&sector[0x98], "%-8.8s", "");

   PUTVAL (sector, 0x0E, 0x1);
   PUTVAL (sector, 0x24, 0x1);
   PUTVAL (sector, 0x18, loadaddr+8);
   PUTVAL (sector, 0x1A, pgmlen+8);

   if (fwrite (sector, 1, secsize, dfd) < 0) goto FAILED;
   if (write_overhead (dfd, overhead) < 0) goto FAILED;

   /*
   ** Initialize remainder of track 0.
   */

   memset (sector, 0, sizeof(sector));
   for (i = 1; i < sectrk; i++)
   {
      if (fwrite (sector, 1, secsize, dfd) < 0) goto FAILED;
      if (write_overhead (dfd, overhead) < 0) goto FAILED;
   }

   /*
   ** Write track 1 loader image.
   */

   PUTVAL (memory, loadaddr, loadaddr+8);
   PUTVAL (memory, loadaddr+2, loadaddr);
   for (i = 0; pgmlen > 0 && i < sectrk; i++)
   {
      if (fwrite (&memory[loadaddr], 1, secsize, dfd) < 0) goto FAILED;
      if (write_overhead (dfd, overhead) < 0) goto FAILED;
      loadaddr += secsize;
      pgmlen -= secsize;
   }
   for (; i < sectrk; i++)
   {
      if (fwrite (&memory[0], 1, secsize, dfd) < 0) goto FAILED;
      if (write_overhead (dfd, overhead) < 0) goto FAILED;
   }

   return (0);

FAILED:
   perror ("mkudisk: write_boot failed");
   return (-1);
}

/***********************************************************************
* write_image
***********************************************************************/

static int
write_image (FILE *dfd, FILE *ufd, int sectrk, int secsize, int overhead)
{
   int done;
   int iosize;
   uint8 block[MAXSECSIZE*2];

   memset (block, 0, sizeof(block));
   iosize = (secsize < 512) ? secsize*2 : secsize;

   done = FALSE;
   while (!done)
   {
      if (fread (block, 1, 512, ufd) == 512)
      {
	 if (overhead)
	 {
	    if (fwrite (block, 1, secsize, dfd) != secsize)
	    	goto IOERROR;
	    if (write_overhead (dfd, overhead) < 0) 
	    	goto IOERROR;
	    if (fwrite (&block[secsize], 1, secsize, dfd) != secsize)
	    	goto IOERROR;
	    if (write_overhead (dfd, overhead) < 0) 
	    	goto IOERROR;
	 }
	 else
	 {
	    if (fwrite (block, 1, iosize, dfd) != iosize)
		   goto IOERROR;
	 }
      }
      else
         done = TRUE;
   }

   return (0);

IOERROR:
   perror ("mkudisk: write_image failed");
   return (-1);
}

/***********************************************************************
* main
***********************************************************************/

int
main (int argc, char **argv)
{
   FILE *dfd;
   FILE *ufd;
   FILE *bfd;
   char *bp;
   char *diskfile;
   char *diskname;
   char *disklabel;
   char *imagefile;
   char *imagename;
   char *bootloader;
   char *up;
   size_t di, disksize;
   int i, j;
   int status;
   int diskndx;
   int verbose;
   int raw;

   verbose = FALSE;
   raw = FALSE;
   diskfile = NULL;
   diskname = NULL;
   imagefile = NULL;
   bootloader = NULL;
   disklabel = NULL;
   imagename = NULL;

   /*
   ** Scan off args 
   */

   for (i = 1; i < argc; i++)
   {
      bp = argv[i];

      if (*bp == '-')
      {
         for (bp++; *bp; bp++) switch (*bp)
         {
	 case 'b':
	    i++;
	    bootloader = argv[i];
	    break;

	 case 'd':
	    i++;
	    diskname = argv[i];
	    break;

	 case 'i':
	    i++;
	    imagename = argv[i];
	    break;

	 case 'l':
	    i++;
	    disklabel = argv[i];
	    break;

	 case 'r':
	    raw = TRUE;
	    break;

	 case 'v':
	    verbose = TRUE;
	    break;

         default:
      USAGE:
	    printf ("usage: mkudisk [-options] diskfile.dsk uniximage.dsk\n");
            printf (" options:\n");
	    printf ("    -b bootloader - Track 1 boot loader object\n");
	    printf ("    -d dskmodel   - Disk model\n");
	    printf ("          ");
	    for (j = i = 0; i < MAXDISKS; i++)
	    {
	       if (j >= 60)
	       {
		  printf ("\n          ");
		  j = 0;
	       }
	       printf ("%s ", disks[i].model);
	       j += strlen (disks[i].model) + 1;
	    }
	    printf ("\n");
	    printf ("    -i imagename  - Disk boot image name\n");
	    printf ("    -l dsklabel   - Disk volume label\n");
            printf ("    -v            - Verbose output\n");
	    return (ABORT);
         }
      }

      else
      {
         if (!diskfile) 
	    diskfile = argv[i];
         else if (!imagefile) 
	    imagefile = argv[i];
         else goto USAGE;
      }

   }

   if (!diskfile) goto USAGE;
   if (!imagefile) goto USAGE;

   if (!diskname) diskname = "DS50";
   if (!disklabel) disklabel = "UNIX";
   if (!imagename) imagename = "LSV";

   for (up = imagename; *up; up++)
      if (islower (*up))
         *up = toupper (*up);

   for (up = disklabel; *up; up++)
      if (islower (*up))
         *up = toupper (*up);

   /*
   ** Check disk model
   */

   for (diskndx = 0; diskndx < MAXDISKS; diskndx++)
   {
      if (!strcmp (diskname, disks[diskndx].model)) break;
   }
   if (diskndx == MAXDISKS)
   {
      printf ("mkudisk: Unknown disk model: %s\n", diskname);
      exit (ABORT);
   }

   /*
   ** Calulate the size of the disk
   */

   disksize = (disks[diskndx].bytsec + disks[diskndx].overhead) * 
	disks[diskndx].sectrk * disks[diskndx].heads * disks[diskndx].cyls;

   /*
   ** Open the disk image file
   */

   if ((dfd = fopen (diskfile, "wb")) == NULL)
   {
      perror ("mkudisk: Can't open diskfile");
      exit (ABORT);
   }

   /*
   ** Open the unix image file
   */

   if ((ufd = fopen (imagefile, "rb")) == NULL)
   {
      perror ("mkudisk: Can't open unix image file");
      fclose (dfd);
      exit (ABORT);
   }

   /*
   ** Open the track 1 boot loader file
   */

   bfd = NULL;
   if (bootloader)
   {
      if ((bfd = fopen (bootloader, "rb")) == NULL)
      {
	 perror ("mkudisk: Can't open boot loader file");
	 fclose (dfd);
	 fclose (ufd);
	 exit (ABORT);
      }
   }

   if (verbose)
   {
      float size;
      size = disksize / 1048576.0;
      printf ("mkudisk: create%sdisk %s on file %s\n",
	      raw ? " raw " : " ", diskname, diskfile);
      printf (
"Geometry: cyls = %d, heads = %d, sec/trk = %d, byte/sec = %d, overhead = %d\n",
		  disks[diskndx].cyls, disks[diskndx].heads,
		  disks[diskndx].sectrk, disks[diskndx].bytsec,
		  disks[diskndx].overhead);
      printf (" Disk size = %6.2f MB\n", size);
      printf (" Volume name: %s\n", disklabel);
      printf (" Image name: %s\n", imagename);
      printf (" Track 1 loader: %s\n", bootloader ? bootloader : "NONE");
      printf (" Unix Image: %s\n", imagefile);
   }

   /*
   ** Write the disk geometry information
   */

   status = ABORT;
   if (!raw)
   {
      if (dskwriteint (dfd, disks[diskndx].cyls) < 0) goto DIE;
      if (dskwriteint (dfd, disks[diskndx].heads) < 0) goto DIE;
      if (dskwriteint (dfd, (disks[diskndx].overhead << 8) |
	  disks[diskndx].sectrk) < 0) goto DIE;
      if (dskwriteint (dfd, disks[diskndx].bytsec) < 0) goto DIE;
   }

   /*
   ** Write Boot
   */

   if (write_boot (dfd, bfd, disklabel, imagename,
		   disks[diskndx].sectrk, disks[diskndx].bytsec,
		   disks[diskndx].overhead) < 0)
      goto DIE;

   /*
   ** Write Unix image
   */

   if (write_image (dfd, ufd, disks[diskndx].sectrk, disks[diskndx].bytsec,
		    disks[diskndx].overhead) < 0)
      goto DIE;

   /*
   ** Write remainder if the disk image
   */

   di = ftell(dfd) - DSKOVERHEAD; 
   if (di > disksize)
   {
      printf ("mkudisk: Image too big for %s disk\n", diskname);
      printf ("   di = %ld, disksize = %ld\n", di, disksize);
      goto DIE;
   }
   if (verbose)
      printf ("   residual = %ld\n", disksize - di);
   for (; di < disksize; di++)
   {
      fputc ('\0', dfd);
   }
   status = NORMAL;
   
DIE:
   fclose (dfd);
   fclose (ufd);
   if (bfd) fclose (bfd);
   return (status);
}
