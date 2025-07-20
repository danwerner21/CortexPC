/***********************************************************************
*
* hexdump.c - Provides a hex dump function.
*
* Changes:
*   12/17/08   DGP   Original.
*
***********************************************************************/

#include <stdio.h>
#include <ctype.h>

/***********************************************************************
* hexdump - Dump formated data in hex.
***********************************************************************/

void
hexdump (file, ptr, offset, size)
FILE *file; char *ptr; int offset; int size;
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
	    if (isprint (*tp))
	       fprintf (file, "%c", *tp);
	    else
	       fprintf (file, ".");
	    tp++;
	    if (tp < ((char *)ptr + size))
	    {
	       if (isprint (*tp))
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
