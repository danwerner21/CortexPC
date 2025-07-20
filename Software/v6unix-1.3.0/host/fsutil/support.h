/***********************************************************************
*
* support.h - Support routines for the TI 990 utilities.
*
* Changes:
*   04/02/14   DGP   Split from other programs.
*
***********************************************************************/
#ifndef __SUPPORT_H__
#define __SUPPORT_H__

/*
** Misc support defines
*/

#define NORMAL		0
#define ABORT		16

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MSB 0
#define LSB 1

#define PUTVAL(m,l,v) \
   { m[(l)+MSB] = (v) >> 8 & 0xFF; m[(l)+LSB] = (v) & 0xFF; }

#define SYSMEMSIZE 32768

#define EOFSYM ':'

#define MAXSECSIZE 512

/*
** Object tags
*/

#define BINIDT_TAG	01
#define IDT_TAG		'0'
#define ABSENTRY_TAG	'1'
#define RELENTRY_TAG	'2'
#define RELEXTRN_TAG	'3'
#define ABSEXTRN_TAG	'4'
#define RELGLOBAL_TAG	'5'
#define ABSGLOBAL_TAG	'6'
#define CKSUM_TAG	'7'
#define NOCKSUM_TAG	'8'
#define ABSORG_TAG	'9'
#define RELORG_TAG	'A'
#define ABSDATA_TAG	'B'
#define RELDATA_TAG	'C'
#define LOADBIAS_TAG	'D'
#define EXTNDX_TAG	'E'
#define EOR_TAG		'F'

#define RELSYMBOL_TAG	'G'
#define ABSSYMBOL_TAG	'H'
#define PGMIDT_TAG	'I'
#define CMNSYMBOL_TAG	'J'
#define COMMON_TAG	'M'
#define CMNDATA_TAG	'N'
#define CMNORG_TAG	'P'
#define CBLSEG_TAG	'Q'
#define DSEGORG_TAG	'S'
#define DSEGDATA_TAG	'T'
#define LOAD_TAG	'U'
#define RELSREF_TAG	'V'
#define CMNGLOBAL_TAG	'W'
#define CMNEXTRN_TAG	'X'
#define ABSSREF_TAG	'Y'
#define CMNSREF_TAG	'Z'

#define LRELSYMBOL_TAG	'i'
#define LABSSYMBOL_TAG	'j'
#define LRELEXTRN_TAG	'l'
#define LABSEXTRN_TAG	'm'
#define LRELGLOBAL_TAG	'n'
#define LABSGLOBAL_TAG	'o'
#define LLOAD_TAG	'p'
#define LRELSREF_TAG	'q'
#define LABSSREF_TAG	'r'

#define CHARSPERREC	66  /* Chars per object record */
#define WORDTAGLEN	5   /* Word + Object Tag length */
#define BINWORDTAGLEN	3   /* Binary Word + Object Tag length */
#define EXTRNLEN	11  /* Tag + SYMBOL + addr */
#define GLOBALLEN	11  /* Tag + SYMBOL + addr */
#define IDTTAGLEN	13  /* Tag + IDT + length */
#define SEQUENCENUM	74  /* Where to put the sequence */
#define SYMLEN		6   /* REF/DEF Symbol length */
#define IDTLEN		8   /* IDT length */

/*
** Data type definitions
*/

#define int8            signed char
#define int16           short
#define int32           int
typedef int             t_stat;                         /* status */
typedef int             t_bool;                         /* boolean */
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32, t_addr;                 /* address */

#if defined (WIN32)                                     /* Windows */
#define t_int64 __int64
#elif defined (__ALPHA) && defined (VMS)                /* Alpha VMS */
#define t_int64 __int64
#elif defined (__ALPHA) && defined (__unix__)           /* Alpha UNIX */
#define t_int64 long
#else                                                   /* default GCC */
#define t_int64 long long
#endif                                                  /* end OS's */
typedef unsigned t_int64        t_uint64, t_value;      /* value */
typedef t_int64                 t_svalue;               /* signed value */

extern int dskreadint (FILE *);
extern int dskwriteint (FILE *, int);

extern int binloader (FILE *, int);

#endif
