/***********************************************************************
*
* support.c - Support routines for the TI 990 utilities.
*
* Changes:
*   04/02/14   DGP   Split from other programs.
*
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>

/***********************************************************************
* dskreadint - Read an integer.
***********************************************************************/

int
dskreadint (FILE *fd)
{
   int r;
   int i;

   r = 0;
   for (i = 0; i < 4; i++)
   {
      int c;
      if ((c = fgetc (fd)) < 0)
      {
	 perror ("dskreadint failed");
	 return (-1);
      }
      r = (r << 8) | (c & 0xFF);
   }
   return (r);
}

/***********************************************************************
* dskwriteint - Write an integer.
***********************************************************************/

int
dskwriteint (FILE *fd, int num)
{
   int status = 0;
   if (fputc ((num >> 24) & 0xFF, fd) < 0) status = -1;
   if (fputc ((num >> 16) & 0xFF, fd) < 0) status = -1;
   if (fputc ((num >> 8) & 0xFF, fd) < 0) status = -1;
   if (fputc (num & 0xFF, fd) < 0) status = -1;
   if (status < 0)
      perror ("dskwriteint failed");
   return (status);
}

