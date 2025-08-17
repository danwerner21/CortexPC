
/* tables.c */

enum optypes
{
   type_1=0,
   type_2,
   type_3,
   type_4,
   type_5,
   type_6,
   type_7,
   type_8,
   type_9,
   type_10,
   type_11,	/* "long jumps" */
   type_12,	/* lmf */
   type_p	/* pseudo op */
};

#define type_opc type_p
#define OPCODE   (-1)

extern void sym_init();


/* symbol.c */

struct symbol {				
	struct symbol		*next;		/* chain to next symbol with same hash code */
	struct symbol		*link;		/* chain to next symbol with next index */
	char			name[8];	/* left-justified, zero-padded */
	short			type;		/* relocatability type */
	short			value;
	short			idx;		/* symbol index in symbol table */
};
typedef struct symbol SYMBOL;

extern SYMBOL	*cursym;
extern char	symbuf[];
extern int	symcount;
extern SYMBOL	*firstsym;
extern SYMBOL	*linksym;

#ifdef __STDC__
extern SYMBOL	*sym_enter(char *name);
extern int	symlook(int flag);
#else
extern SYMBOL	*sym_enter();
extern int	symlook();
#endif
extern void	sym_iter();

/* main.c */

/* error types */

#define erra	 0	/* previously defined absolute value required */
#define errb	 1	/* short branch out of range */
#define errc	 2	/* illegal or missing opcode */
#define errd	 3	/* illegal data in .bss segment */
#define erre	 4	/* eof or END within ifx */
#define errg	 5	/* garbage character */
#define errh	 6	/* halfword value too large */
#define erri	 7	/* improper nesting of .if and .endif */
#define errl	 8	/* missing label */
#define errm	 9	/* multiply defined symbol */
#define errn	10	/* number syntax */
#define erro	11	/* org to illegal address */
#define errp	12	/* symbol value changed between passes */
#define errq	13	/* missing quote */
#define errr	14	/* relocation error */
#define errs	15	/* missing symbol */
#define erru	16	/* undefined symbol */
#define errv	17	/* illegal value (e.g. register >15) */
#define errx	18	/* syntax error */
#define errz	19	/* division by zero */
#define errt	20	/* numeric label overflow */
#define errf	21	/* illegal map file */

#ifdef __STDC__
extern void	cerror(int type);
extern void	xerror(int type);
#else
extern void	cerror();
extern void	xerror();
#endif
extern int 	errcnt;

extern int	fd_in;			/* fd of current input file */

extern int	nextfile();

/* lexer.h */

/* lexical tokens - returned by token() */

#define	GARBAGE	0	/*  unknown character */
#define	PLUS	'+'
#define	MINUS	'-'
#define	OR	'|'
#define	AND	'&'
#define	STAR	'*'
#define	SLASH	'/'
#define	SPACE	' '	/* blanks and/or tabs */
#define	COMMA	','
#define	LPAREN	'('
#define	RPAREN	')'
#define	QUOTE	'\''
#define	DQUOTE	'"'
#define	AT	'@'
#define DOT	'.'
#define COLON	':'
#define SEMI	';'
#define EQU	'='

#define BINOP	16	/* last binary operator */
#define	EOL	17	/* end of line */
#define	CON	18	/* constant - value in conbuf */
#define	STRING	19	/* string & length in strbuf & strlen */
#define	IDENT	20	/* symbol - name in symbuf */
#define LABEL   21	/* symbol - name in symbuf, followed by a colon */
#define COMMENT 22	/* // found, skip rest of line */
#define NLABEL	23	/* numeric label, e.g. 1f: */
#define NIDENT	24	/* numeric identifier, e.g. 1f */

extern int	line;			/* line counter */
extern int	nexttoken;		/* look-ahead token */
extern int 	ucase;			/* if true, convert uppercase to lowercase symbols */
extern int 	uflag;			/* if true, make undefined symbols external */
extern char	strbuf[71];		/* returned value of last string */
extern long	conbuf;			/* returned value of last constant */
extern int	strsiz;			/* returned length of last string */

extern int get_line();
extern int token();
#ifdef __STDC__
extern int tok(int lookup);
#else
extern int tok();
#endif
extern int getsym();
extern int eol();
extern void sscan();
extern void rscan();

/* assem.c */

#define PBYTE	1
#define PEVEN	2
#define PIF	3
#define PENDIF	4
#define PGLOBL	5
#define PTEXT	6
#define PDATA	7
#define PBSS	8
#define PCOMM	9

/* symbol types */

#define	SUNDEF	 00		/* undefined */
#define	SABS	 01		/* absolute */
#define	STEXT	 02		/* text */
#define	SDATA	 03		/* data */
#define	SBSS	 04		/* bss */
#define	SSEG	 07		/* mask for segment */
#define	SEXT	040		/* external (global) symbol */

/* segment relocation bits -- used in a.out relocation sections */

#define	RUNDEF	(-1)		/* undefined -- internal only */
#define	RABS	 00		/* absolute */
#define	RTEXT	 02		/* text = PURE */
#define	RDATA	 04		/* data = IMPUR */
#define	RBSS	 06		/* bss */
#define	REXT	010		/* external reference -- bits 15-4 contain symbol number */
#define	RSEG	017		/* Mask for segment */

/* branch tab for synthetic relational far branches */
struct branch {
	int opc1;
	int opc2;
	int reverse;
};

extern struct branch branchtab[];

/* table for numeric labels, e.g. 1f */
struct nlabel {
	char num;
	char seg;
	int loc;
};

extern struct nlabel nlabtab[];

/* segment information */

#define	OBSIZE	512		/* size of output buffers (must be even) */

extern struct segment {
	int loc;		/* location counter */
	int maxloc;		/* highest address so far */
	/* buffers below, only used for .text and .data segments */
	int nchar;		/* number of chars in buffers */
	int tseek;		/* seek address for text buffer */
	int rseek;		/* seek address for relocation bits buffer */
	char tbuf[OBSIZE];	/* text buffer */
	char rbuf[OBSIZE];	/* relocation bits buffer */
} text, data, bss, comm;

typedef struct segment SEGMNT;

extern SEGMNT *curseg;		/* current segment */
extern SYMBOL *curcomm;		/* current COMN or STRUC name */
extern SYMBOL *curlab;		/* current label */

extern int	   currel;		/* current relocatability */

extern int	pass;		/* pass number */
extern int	pass_gen;	/* generate code in this pass ?    */
extern int	pass_lst;	/* generate listing in this pass ? */

extern int  assemble();
#ifdef __STDC__
extern void do_pseudo(int op);
extern void do_machine(int op);
#else
extern void do_pseudo();
extern void do_machine();
#endif

/* list.c */

extern void putline();
extern void lstinit();
extern void lstloc();
#ifdef __STDC__
extern void lstb(int val, int rel);
extern void lstw(int val, int rel);
extern void lstaddr(int val, int rel);
#else
extern void lstb();
extern void lstw();
extern void lstaddr();
#endif

/* output.c */

extern int	ofile;		/* fd of output file */

extern void seginit();
extern void outhdr();
extern void symout();
extern void relocate_0407();
extern void nlabinit();

#ifdef __STDC__
extern int  reloc(int val, int rel);
extern void putwd(int val, int seg);
extern void putb(int val);
extern void putstr(int len);
extern void org(int loc);
extern void deflab(int rel, int val);
extern void align(int boundary);
extern void oflush(SEGMNT *sp);
extern void defnlab(long num, char seg, int loc);
extern struct nlabel *getnlab(long num);
#else
extern int  reloc();
extern void putwd();
extern void putb();
extern void putstr();
extern void org();
extern void deflab();
extern void align();
extern void oflush();
extern void defnlab();
extern struct nlabel *getnlab();
#endif

/* expr.c */

struct expr {
	int rel;		/* relocatability of expression */
	int val;		/* value of expression */
};

typedef struct expr EXPR;

extern EXPR res;

extern int expr();


