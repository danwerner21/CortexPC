/***********************************************************************
*
* binloader.c - Binary loader for the TI 990 simulator.
*
* Changes:
*   05/29/03   DGP   Original.
*   07/02/03   DGP   Added compressed binary support.
*   12/30/03   DGP   Moved loader code to loadrec function.
*   04/12/04   DGP   Changed to allow non-80 byte records.
*   04/14/04   DGP   Ignore extended object tags; Add checksum support.
*   08/03/11   DGP   Generate error if program is too big.
*   02/12/14   DGP   Fixed compressed (binary) load.
*   03/06/14   DGP   Added no checksum tag.
*   07/14/15   DGP   Adapted for track 1 boot loader.
*
***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>

#include "support.h"

extern uint8 memory[SYSMEMSIZE];

static int pgmlen;

static int
loadrec (uint8 *inbuf, int loadaddr, int *curraddr, int binarymode)
{
   static int wordtaglen = WORDTAGLEN;
   uint8 *op = (uint8 *)inbuf;
   int i, j;
   int16 cksum;

   if (*op == EOFSYM)
   {
      wordtaglen = WORDTAGLEN;
      return (EOF);
   }

   cksum = 0;
   for (i = 0; i < 80; i++)
   {
      int tmp;
      uint16 wdata;
      uint8 otag;
      uint8 item[16];

      otag = *op++;
      if (binarymode)
      {
	 wdata = (*op << 8) | *(op+1);
      }
      else
      {
	 cksum += otag;
	 memcpy (item, op, 4);
	 item[4] = '\0';
	 if (otag != CKSUM_TAG)
	    for (j = 0; j < 4; j++) cksum += item[j];
	 sscanf ((char *)item, "%4X", &tmp);
	 wdata = tmp & 0xFFFF;
      }
      wdata &= 0xFFFF;

      switch (otag)
      {
      case BINIDT_TAG: /* Binary IDT */
	 wordtaglen = BINWORDTAGLEN;
	 wdata = (*op << 8) | *(op+1);

      case IDT_TAG:
	 pgmlen = wdata;
	 wdata += loadaddr;
	 if (wdata >= SYSMEMSIZE)
	 {
	    fprintf (stderr, "binloader: Program too large for memory");
	    return (-21);
	 }
	 op += wordtaglen-1;
	 if (!binarymode)
	    for (j = 0; j < IDTLEN; j++) cksum += *op++;
	 else
	    op += IDTLEN;
	 break;

      case RELORG_TAG:
	 wdata += loadaddr;
      case ABSORG_TAG:
	 *curraddr = wdata;
	 op += wordtaglen-1;
	 break;

      case RELDATA_TAG:
	 wdata += loadaddr;
      case ABSDATA_TAG:
	 PUTVAL (memory, *curraddr, wdata);
	 *curraddr += 2;
	 op += wordtaglen-1;
	 break;

      case RELENTRY_TAG:
	 wdata += loadaddr;
      case ABSENTRY_TAG:
	 op += wordtaglen-1;
	 break;

      case CKSUM_TAG:
         cksum += (int16)wdata;
	 if (cksum != 0)
	 {
	    fprintf (stderr, "binloader: Checksum error");
	    return (-20);
	 }
      case NOCKSUM_TAG:
	 op += wordtaglen-1;
         break;

      case RELEXTRN_TAG:
      case ABSEXTRN_TAG:
      case ABSGLOBAL_TAG:
      case RELGLOBAL_TAG:
      case RELSYMBOL_TAG:
      case ABSSYMBOL_TAG:
      case LOAD_TAG:
      case RELSREF_TAG:
      case ABSSREF_TAG:
	 op += wordtaglen-1;
	 if (!binarymode)
	    for (j = 0; j < SYMLEN; j++) cksum += *op++;
	 break;

      case EXTNDX_TAG:
	 op += wordtaglen-1;
	 PUTVAL (memory, *curraddr, wdata);
	 *curraddr += 2;
	 if (!binarymode)
	    for (j = 0; j < 4; j++) cksum += *op++;
         break;

      case EOR_TAG:
         i = 81;
	 break;

      default: 
	 fprintf (stderr, "binloader: Illegal object tag: %c(%02X), %9.9s",
	 	  otag, otag, &inbuf[71]);
	 return (-22);
      }
   }
   return (0);
}

int
binloader (FILE *fd, int loadpt)
{
   int status;
   int done;
   int binarymode = FALSE;
   int loadaddr = 0x00A8;
   int curraddr = 0x00A8;
   uint8 inbuf[82];

   if (loadpt > 0)
   {
      loadaddr = loadpt;
      curraddr = loadpt;
   }

   status = fgetc (fd);
   if (status == BINIDT_TAG)
      binarymode = TRUE;
   ungetc (status,fd);

   done = FALSE;
   pgmlen = -1;
   while (!done)
   {
      if (binarymode)
      {
         if ((fread (inbuf, 1, 81, fd) != 81) || feof (fd))
	 {
	    done = TRUE;
	    break;
	 }
      }
      else
      {
	 if (fgets ((char *)inbuf, 82, fd) == NULL)
	 {
	    done = TRUE;
	    break;
	 }
      }

      if ((status = loadrec (inbuf, loadaddr, &curraddr, binarymode)) != 0)
      {
	 if (status == EOF) break;
	 fclose (fd);
	 return (-1);
      }
   }

   return (pgmlen);
}
