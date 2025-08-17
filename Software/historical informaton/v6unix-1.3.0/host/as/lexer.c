/*
 *		Assembler input & lexical scan routines
 */

#include "as.h"
#include <unistd.h>

char*	getstr();
int	getsym();
#ifdef __STDC__
int	getnum(int base);
#else
int	getnum();
#endif

/*
 * Parser information
 */

char	linebuf[74];	/* input line buffer */
int	line;		/* line counter */

int	nexttoken;	/* look-ahead token */
char	strbuf[71];	/* returned value of last string */
long	conbuf;		/* returned value of last constant */
int	strsiz;		/* returned length of last string */


/*
 * Character types
 */
#define	C_ALPHA		0x80	/* Alphabetic (i.e. legal within symbol) */
#define	C_HEXDIGIT	0x40	/* Hexadecimal digit [0-9 a-f A-F] */
#define C_DIGIT		0x20	/* Decimal digit [0-9] */
#define	C_TOKEN		0x10	/* Other special characters */
#define	C_VALUE		0x0f	/* C_HEXDIGIT - numerical value of digit */

/*
 * Character type table
 */
unsigned char ctab[128] = {
	0x10,	0,	0,	0,	0,	0,	0,	0,
	0,	0x10,	0x10,	0,	0,	0x10,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0x10,	0x10,	0x10,	0,	0x10,	0,	0x10,	0x10,
	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x80,	0x10,
	0x60,	0x61,	0x62,	0x63,	0x64,	0x65,	0x66,	0x67,
	0x68,	0x69,	0x10,	0x10,	0x10,	0x10,	0x10,	0,
	0x10,	0xca,	0xcb,	0xcc,	0xcd,	0xce,	0xcf,	0x80,
	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,
	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,
	0x80,	0x80,	0x80,	0,	0,	0,	0,	0x80,
	0,	0xca,	0xcb,	0xcc,	0xcd,	0xce,	0xcf,	0x80,
	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,
	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,
	0x80,	0x80,	0x80,	0,	0x10,	0,	0x80,	0
};

int xlate_c(c)
	int c;
{
	switch (c) {
	case 'a': return '\a';
	case 'b': return '\b';
	case 'f': return '\f';
	case 'n': return '\n';
	case 'r': return '\r';
	case 't': return '\t';
	case 'v': return '\v';
	case '0': return '\0';
	default:
		return c;
	}
}

/*
 * Read next input line, truncating to 71 chars
 *	- resets scan pointer to start of line
 *	- initializes listing buffer
 *	- returns 0 at end-of-file, '\n' otherwise
 */

char *scanp;				/* input scan cursor */
int   docount;

static char	ibuf[512];
static char	*next;
static int	nleft;

int get_line()
{
	char *nx, *p;
	int   nl, ncol;

	scanp = p = linebuf;
	lstinit();

	ncol = 71;
	for (nx = next, nl = nleft; ;) {
		if (--nl <= 0) {
			while ((nl = read(fd_in, ibuf, 512)) <= 0)
				if (!nextfile()) {
					nleft = 0;
					return(0);
				}
			nx = ibuf;
		}
		if ((*p = *nx++) == '\n') {
			*p    = '\0';
			next  = nx;
			nleft = nl;
			line++;
			return('\n');
		}
		if (--ncol > 0)
			p++;
	}
}

/*
 * Return next lexical token from input line
 *	- if constant, value is in conbuf
 *	- if symbol & lookup flag is true, symbol pointer is in cursym
 */
int tok(lookup)
	int lookup;
{
	int ret, c;
	int ctype;
	char *p;

	/* first check look-ahead token */
	if ((ret = nexttoken)) {
		nexttoken = 0;
		return (ret);
	}

	/* Skip whitespace */
	p = scanp;
	while ((c = *p) == ' ' || c == '\t') p++;
	scanp = p;
  
	for(;;) {

		p = scanp;
		c = *p;
		ctype = ctab[c];

		/* first character "alphabetic" */
		if ( ctype & C_ALPHA) {
			if (c=='.' && !(ctab[(int)*(p+1)] & C_ALPHA) ) {
				++scanp;
				return (DOT);
			}
			getsym();
			if (lookup)
				symlook(2);
			if (*scanp==COLON) {
				++scanp;
				return (LABEL);
			}
			else
				return (IDENT);
		}
		/*
		 * first character numeric -- numeric constant or label
		 */
		if (ctype & C_DIGIT) {
			if (c == '0') {
				if (p[1]=='x') {
					scanp = p+2;
					conbuf = getnum(16);
				} else {
					scanp = p;
					conbuf = getnum(8);
					goto testnlab;
				}
			} else {
				conbuf = getnum(10);
testnlab:
				if (conbuf>=0 && conbuf<=9) {
					c = *scanp;
					switch(c) {
					case 'b':
						conbuf = -conbuf;
					case 'f':
						++scanp;
						return(NIDENT);
					case COLON:
						++scanp;
						return(NLABEL);
					}
				}
			}
			return(CON);
		}
		/*
		 * first character QUOTE
		 */
		if (c == QUOTE) {
			c = *++p;
			conbuf = (c == '\\') ? xlate_c(*++p) : c;
			if (p[1] != QUOTE) {
				conbuf <<= 8;
				c = *++p;
				conbuf += (c == '\\') ? xlate_c(*++p) : c;
			}
			if (*++p != QUOTE)
				xerror(errx);
			scanp = p + 1;
			return(CON);
		}
		/*
		 * first character DQUOTE
		 */
		if (c == DQUOTE) {
			scanp = p+1;
			conbuf = (long) getstr();
			return (STRING);
		}

		/* special characters */
		scanp = ++p;
		if (!ctype)
			xerror(errg);

		switch (c) {

			/* white space */
			case ' ':
			case '\t':
				while ((c = *p) == ' ' || c == '\t')
					p++;
				scanp = p;
				continue;

			case '/':
				if (*p=='/') {
					while (*p++);
					scanp = --p;
					return (COMMENT);
				} else
					return (c);

			/* don't scan past end of line */
			case 0:
			case '\n':
			case '\r':
				scanp = --p;
				return (EOL);

			/* single-character tokens */
			default:
				return (c);

		}
	}
}

int token()
{
	return tok(1);
}

/*
 * Scan a decimal or hexadecimal number (with optional sign)
 *	 from input line
 */
int getnum(base)
	int base;
{
	char	*p;
	int		c, n, sign;

	p = scanp;
	sign = 1;
	if ((c = *p) == '-') {
		sign = -1;
		c = *++p;
	}
	else if (c == '+')
		c = *++p;
	if (((c = ctab[c])&C_HEXDIGIT) == 0 || (n = c&C_VALUE) >= base)
		xerror(errn);
	while ((c = ctab[(int)*++p])&C_HEXDIGIT && (c &= C_VALUE) < base) {
		n *= base;
		n += c;
	}
	scanp = p;
	return (n*sign);
}

/*
 * Scan a character string from input line until ending quote
 *	- translate pairs of quotes to single quote
 *	- leave string in strbuf, and length in strlen
 * Returns a pointer to strbuf
 */

char* getstr()
{
	char	*s, *p, *e = strbuf + sizeof(strbuf) - 1;
	int	c;

	s = strbuf;
	for (p = scanp; s < e; p++) {
		if ((c = *p) == '\\') {
			c = xlate_c( *++p );
		}
		else if (c == '"')
			break;
		else if (c == '\n')
			xerror(errq);
		*s++ = c;
	}
	if (c != '"') xerror(errq);
	*s = 0;
	strsiz = s - strbuf;
	scanp = p + 1;
	return (strbuf);
}

/*
 * Scan a symbol from the input line into symbuf
 *	truncate long symbols to 8 characters (without warning)
 *
 *	returns	0:	no legal symbol found
 *		1:	symbol found
 */

int	ucase = 0;

int getsym()
{
	int c, ctype;
	char *p, *s;

	p = scanp;
	c = *p;
	ctype = ctab[c];
	if (!(ctype & C_ALPHA))
		return(0);

	s = symbuf;
	s[0] = s[1] = s[2] = s[3] = 0;
	s[4] = s[5] = s[6] = s[7] = 0;
	do {
		if (s < &symbuf[8]) {
			if (ucase && (c>='A' && c<='Z'))
				c += 'a'-'A';
			*s++ = c;
		}
		c = *++p;
		ctype = ctab[c];
	} while ( ctype & (C_ALPHA|C_DIGIT) );

	scanp = p;
	return(1);
}

/*
 * Determine whether scan pointer is at end of line
 */
int eol()
{
	int c;

	c = ctab[(int)*scanp];
	return( c == EOL || c == SPACE );
}

/*
 * Save / restore scan pointer (for backtracking)
 */

static char	*scansave;
static int   nxt_save;

void sscan()
{
	nxt_save = nexttoken;
	scansave = scanp;
}

void rscan()
{
	nexttoken = nxt_save;
	scanp = scansave;
}
