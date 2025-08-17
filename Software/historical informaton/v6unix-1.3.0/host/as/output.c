/*
 *		Assembler output & location counter routines
 */

#include <unistd.h>
#include "as.h"

int		ofile;		/* fd of output file */
int		symseek;	/* offset of symbol table in a.out */

/* initialize segments (at the beginning of each pass) */

void seginit()
{
	data.loc = data.maxloc = 0;
	text.loc = text.maxloc = 0;
	bss.loc  = bss.maxloc  = 0;

	curseg = &text;
	currel = RTEXT;
}

/* write out the a.out header and initialize output buffers
   (at the beginning of code-generation pass(es)) */

/*
 * a.out file header
 */
static struct {
	short mword;		/* 'magic word'				*/
	short tsize;		/* size of text (rounded to word)	*/
	short dsize;		/*		"  data		"	*/
	short bsize;		/*		"  bss		"	*/
	short ssize;		/* size of symbol table			*/
	short entrypt;		/* entry point (NOT IMPLEMENTED)	*/
	short unused;
	short rflag;		/* relocation bits suppressed		*/
} hdr;

/*
 * put a word to outfile in big-endian order.
 */
void putshort(n)
	short n;
{
	char hi,lo;
	hi = n>>8;
	lo = n&255;
	n = write(ofile, &hi, 1);
	n = write(ofile, &lo, 1);
}

void outhdr()
{
	int n;

	if (text.maxloc < text.loc)		text.maxloc = text.loc;
	if (data.maxloc < data.loc)		data.maxloc = data.loc;
	if (bss.maxloc < bss.loc) 		bss.maxloc = bss.loc;

	hdr.mword	= 0407;
	hdr.tsize	= (text.maxloc+01) & ~01;
	hdr.dsize	= (data.maxloc+01) & ~01;
	hdr.bsize	= (bss.maxloc+01)  & ~01;
	hdr.ssize	= 12*symcount;
	hdr.entrypt	= 0;
	hdr.unused 	= 0;
	hdr.rflag  	= 0;

	putshort(hdr.mword);
	putshort(hdr.tsize);
	putshort(hdr.dsize);
	putshort(hdr.bsize);
	putshort(hdr.ssize);
	putshort(hdr.entrypt);
	putshort(hdr.unused);
	putshort(hdr.rflag);

	text.tseek  = n = sizeof hdr;
	data.tseek  = (n += hdr.tsize);
	text.rseek  = (n += hdr.dsize);
	data.rseek  = (n += hdr.tsize);
	symseek     = (n += hdr.dsize);

	text.nchar  = 0;
	data.nchar  = 0;
}

/*
 * Flush the output buffer for the specified segment
 */
void oflush(sp)
	SEGMNT *sp;
{
	int n, off, d;

	/*
	 * If text in buffer begins on an odd byte boundary, adjust offset
	 *	and length for writes
	 */
	off = sp->tseek & 01;
	if ((n = sp->nchar-off) == 0)
		return;

	lseek(ofile, (long)sp->tseek, 0);
	d = write(ofile, sp->tbuf+off, n);
	lseek(ofile, (long)sp->rseek, 0);
	d = write(ofile, sp->rbuf+off, n);
	sp->tseek += n;
	sp->rseek += n;
}

/*
 * Define the symbol in curlab to have the given relocatability and value
 *	- converts relocation bits to symbol type
 *	- checks for illegal redefinitions
 *	- redefinition of absolute symbols is always permitted
 * 	- allow redefintion of common symbols to data symbols (the linker
 * 	  will resolve this)
 */
void deflab(rel, val)
int rel, val;
{
	int type, otype;

	if (!curlab)
		return;

	/*
	 * convert relocation bits to symbol type
	 */
	if (rel&REXT)
		type = (rel & ~RSEG)<<4;
	else
		type = (rel>>1)+1;

	/*
	 * check for illegal redefinitions
	 */
	otype = curlab->type & SSEG;
	if (pass == 0) {
		if (otype!=SUNDEF && (otype!=SABS || type!=SABS))
			cerror(errm);
	}
	else if (otype != type) {
		cerror(errm);
	}
	else if (pass_gen && type!=SABS && !(curlab->type&SEXT) && curlab->value!=val) {
		cerror(errp);
	}

	curlab->type = (curlab->type & SEXT) | type;
	curlab->value = val;
}

/*
 * Emit a string of <len> characters
 */
void putstr(len)
	int len;
{
	SEGMNT	*sp;
	register int	i, n;

	if ((sp = curseg) != &data && sp != &text)
		xerror(errd);

	sp->loc += len;

	if (pass_gen) {
		n = sp->nchar;
		for (i = 0; i<len; i++) {
			if (n >= OBSIZE) {
				sp->nchar = n;
				oflush(sp);
				n = 0;
			}
			sp->tbuf[n] = strbuf[i];
			sp->rbuf[n] = (n&1) ? RABS : 0;
			++n;
		}
		sp->nchar = n;
	}

	if (pass_lst)
		for (i = 0; i<len; i++)
			lstb(strbuf[i], RABS);
}

/*
 * Relocate a value in segment rel
 */
int reloc(val, rel)
	int val, rel;
{
	switch (rel) {
	case RBSS:	val += hdr.dsize;
	case RDATA:	val += hdr.tsize;
	}
	return val;
}

/*
 * Emit a word - current location counter must be even
 */
void putwd(val, rel)
	int val, rel;
{
	SEGMNT	*sp;
	int		n;

	if ((sp = curseg) != &text && sp != &data)
		xerror(errd);

	sp->loc += 2;

	if (pass_gen) {

		val = reloc(val, rel);

		if ((n = sp->nchar) >= OBSIZE) {
			oflush(sp);
			n = 0;
		}
		sp->tbuf[n] = val>>8;
		sp->tbuf[n+1] = val&255;
		sp->rbuf[n] = rel>>8;
		sp->rbuf[n+1] = rel&255;
		sp->nchar = n+2;
	}

	if (pass_lst)
		lstw(val, rel);
}

/*
 * Emit a byte
 */
void putb(val)
	int val;
{
	SEGMNT	*sp;
	int		n;

	if ((sp = curseg) != &data && sp != &text)
		xerror(errd);

	sp->loc++;

	if (pass_gen) {
		if ((n = sp->nchar) >= OBSIZE) {
			oflush(sp);
			n = 0;
		}
		sp->tbuf[n] = val;
		sp->rbuf[n] = (n&1) ? RABS : 0;
		sp->nchar = ++n;
	}
}

/*
 * Align generated code to the specified boundary by padding with zeroes
 */
void align(boundary)
	int boundary;
{
	SEGMNT	*sp;
	int		n;

	n = boundary - 1;
	if ((sp = curseg) != &text && sp != &data) {
		sp->loc = (sp->loc + n) & ~n;
		return;
	}
	while (sp->loc&n)
		putb(0);
}

/*
 * Move the current location counter to the specified location
 */
void org(loc)
	int loc;
{
	register SEGMNT	*sp;
	register int	off;

	sp = curseg;
	if ((off = loc - sp->loc) == 0)
		return;
	if (off < 0)
		xerror(erro);

	if (pass_gen && (sp == &data || sp == &text)) {
		while (sp->loc <  loc) putb(0);
	}
	else
		sp->loc = loc;

	if (sp->maxloc < sp->loc)
		sp->maxloc = sp->loc;
}

/*
 * Append symbol table to object file
 */
void symout()
{
	register SYMBOL *sp;
	int d;

	lseek(ofile, (long)symseek, 0);
	for (sp = firstsym; sp; sp = sp->link) {

		/* relocate */
		switch (sp->type & SSEG) {

		case SUNDEF:
			if( uflag ) sp->type |= SEXT;
			break;

		case SBSS:
			sp->value += hdr.dsize;
		case SDATA:
			sp->value += hdr.tsize;

		}
		d = write(ofile, sp->name, 8);
		putshort(sp->type);
		putshort(sp->value);
	}
}
