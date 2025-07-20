/*
 *		Assembler listing routines
 */

#include <stdio.h>
#include "as.h"


/* buffer for listing */
struct {
	char	l_loc[5];		/* location counter */
	char	l_fill1[1];		/* space */
	char	l_data[17];		/* location contents */
	char	l_fill2[1];		/* space */
	char	l_term[1];		/* null (followed by source line) */
} lstbuf;

char		*listp;			/* current output position in l_data */
extern char	linebuf[];		/* source line */


/* initialize listing buffer */
void lstinit()
{
	char	*p;
	int	n;

	p = lstbuf.l_loc;
	for (n = sizeof(lstbuf); n--;)
		*p++ = ' ';

	lstbuf.l_term[0] = 0;
	listp = &lstbuf.l_data[0];
}

/* convert relocation type to readable value */
char lstreloc(rel)
	int rel;
{
	switch (rel & RSEG) {

		case RABS:	return('a');
		case RTEXT:	return('t');
		case RDATA:	return('d');
		case RBSS: 	return('b');
		case REXT:	return('U');
		default:	return('?');
	}
}

/* list number in hex with leading zeroes */
void lstx(value, nibbles, buf, rel)
	int value, nibbles, rel;
	char *buf;
{
	char	*p;

	for (p = buf+nibbles; --p >= buf; ) {
		*p = "0123456789ABCDEF"[value&0xf];
		value >>= 4;
	}
	buf[nibbles] = lstreloc(rel);
}

/* list a byte of data */
void lstb(val, rel)
	int val, rel;
{
	if (listp < &lstbuf.l_data[15])
		lstx(val, 2, listp, rel);
	else
		lstbuf.l_data[17] = '>';
	listp += 2;
}

/* list a word of code/data */
void lstw(val, rel)
	int val, rel;
{
	if (listp < &lstbuf.l_data[17])
		lstx(val, 4, listp, rel);
	else
		lstbuf.l_data[17] = '>';
	listp += 6;
}

#include <unistd.h>

/* print out listing line */
void putline()
{
	if (pass_lst) {
		printf("%s", (char *)&lstbuf);
		printf(" %05d ", line);
		printf(" %s\n", linebuf);
	}
}


/* list location counter value */
void lstloc()
{
	int loc;
	
	loc = reloc(curseg->loc, currel);
	lstx(loc, 4, lstbuf.l_loc, currel);
}


