/*
 * C second pass -- tables
 */
#include "c1.h"
/*
 * Operator dope table-- see description in c0.
 * BINARY	    01
 * LVALUE	    02
 * RELAT	    04
 * ASSGOP	   010
 * LWORD	   020
 * RWORD	   040
 * COMMUTE	  0100
 * RASSOC	  0200
 * LEAF		  0400
 * CNVRT	 01000
 */
int opdope[] = {
	000000,	/* EOFC (0) */
	000000,	/* ; */
	000000,	/* { */
	000000,	/* } */
	036000,	/* [ */
	002000,	/* ] */
	036000,	/* ( */
	002000,	/* ) */
	014201,	/* : */
	007001,	/* , */
	000000,	/* field selection (10) */
	000000,	/* reverse field selection */
	000001,	/* temporary field selection */
	000001,	/* int->ptr */
	000001,	/* ptr->int */
	000001,	/* long->ptr */
	000001,	/* field assignment */
	000001,	/* >> unsigned */
	000001,	/* >>= unsigned */
	000000,	/* keyword */
	000400,	/* name (20) */
	000400,	/* short constant */
	000400,	/* string */
	000400,	/* float */
	000400,	/* double */
	000400,	/* long const */
	000400,	/* long const <= 16 bits */
	000400,	/* autoi, *r++ */
	000400,	/* autod, *--r */
	000400,	/* () empty arglist */
	034213,	/* ++pre (30) */
	034213,	/* --pre */
	034213,	/* ++post */
	034213,	/* --post */
	034220,	/* !un */
	034202,	/* &un */
	034220,	/* *un */
	034200,	/* -un */
	034220,	/* ~un */
	036001,	/* . (structure reference) */
	030101,	/* + (40) */
	030001,	/* - */
	032101,	/* * */
	032001,	/* / */
	032001,	/* % */
	026061,	/* >> */
	026061,	/* << */
	020161,	/* & */
	016161,	/* | */
	016161,	/* ^ */
	036001,	/* -> (50) */
	001000,	/* int -> double */
	001000,	/* double -> int */
	000001,	/* && */
	000001,	/* || */
	030001, /* &~ */
	001000,	/* double -> long */
	001000,	/* long -> double */
	001000,	/* integer -> long */
	000000,	/* long -> integer */
	022005,	/* == (60) */
	022005,	/* != */
	024005,	/* <= */
	024005,	/* < */
	024005,	/* >= */
	024005,	/* > */
	024005,	/* <p */
	024005,	/* <=p */
	024005,	/* >p */
	024005,	/* >=p */
	012213,	/* += (70) */
	012213,	/* -= */
	012213,	/* *= */
	012213,	/* /= */
	012213,	/* %= */
	012253,	/* >>= */
	012253,	/* <<= */
	012253,	/* &= */
	012253,	/* |= */
	012253,	/* ^= */
	012213,	/* = (80) */
	030001, /* & for tests */
	032001,	/*  * (long) */
	032001,	/*  / (long) */
	032001,	/* % (long) */
	012253,	/* &= ~ */
	012213,	/* *= (long) */
	012213,	/* /= (long) */
	012213,	/* %= (long) */
	000000,	/* (89) */
	014201,	/* question '?' (90) */
	026061,	/* long << */
	012253,	/* long <<= */
	000101,	/* max */
	000101,	/* maxp */
	000101,	/* min */
	000101,	/* minp */
	000001,	/* , */
	000000,	/* call1 */
	000000,	/* call2 */
	036001,	/* call (100) */
	036000,	/* mcall */
	000000,	/* goto */
	000000,	/* jump cond */
	000000,	/* branch cond */
	000400,	/* set nregs */
	000000, /* load */
	030001,	/* ptoi1 */
	000000,	/* (108) */
	000000,	/* int->char */
	000000,	/* force r0 (110) */
	000000,	/* branch */
	000000,	/* label */
	000000,	/* nlabel */
	000000,	/* rlabel */
	000000,	/* structure assign */
	000001,	/* struct assignment setup */
	032001,	/* unsigned / */
	032001,	/* unsigned % */
	012213,	/* unsigned /= */
	012213,	/* unsigned %= (120) */
	032001, /* unsigned long * */
	032001, /* unsigned long / */
	032001, /* unsigned long % */
	012213, /* unsigned long *= */
	012213, /* unsigned long /= */
	012213, /* unsigned long %= */
	01000,  /* unsigned long -> float(double) */
	026061, /* unsigned long >> */
	012253, /* unsigned long >>= */
	030001,	/* x - &name (130) */
};

char	*opntab[] = {
	0,			/* 0 */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	":",
	",",
	"field select",		/* 10 */
	0,
	0,
	"int->ptr",
	"ptr->int",
	"long->ptr",
	"field assign",
	">>",
	">>=",
	"keyword",
	"name",			/* 20 */
	"short constant",
	"string",
	"float",
	"double",
	"long constant",
	"long constant",
	"*r++",
	"*--r",
	"()",
	"++pre",		/* 30 */
	"--pre",
	"++post",
	"--post",
	"!un",
	"&",
	"*",
	"-",
	"~",
	".",
	"+",			/* 40 */
	"-",
	"*",
	"/",
	"%",
	">>",
	"<<",
	"&",
	"|",
	"^",
	"->",			/* 50 */
	"int->double",
	"double->int",
	"&&",
	"||",
	"&~",
	"double->long",
	"long->double",
	"integer->long",
	"long->integer",
	"==",			/* 60 */
	"!=",
	"<=",
	"<",
	">=",
	">",
	"<p",
	"<=p",
	">p",
	">=p",
	"+=",			/* 70 */
	"-=",
	"*=",
	"/=",
	"%=",
	">>=",
	"<<=",
	"&=",
	"|=",
	"^=",
	"=",			/* 80 */
	"& for tests",
	"*",
	"/",
	"%",
	"&= ~",
	"*=",
	"/=",
	"%=",
	0,
	"?",			/* 90 */
	"inc",
	"dec",
	"\\/",
	"\\/",
	"/\\",
	"/\\",
	",",
	"call1",
	"call2",
	"call",			/* 100 */
	"mcall",
	"goto",
	"jump cond",
	"branch cond",
	"set nregs",
	"load value",
	"ptr->integer",
	0,
	"int->char",
	"force register",	/* 110 */
	"branch",
	"label",
	"nlabel",
	"rlabel",
	"=structure",
	"= (struct setup)",
	"/",
	"%",
	"/=",
	"%=",			/* 120 */
	"*",			/* unsigned long */
	"/",			/* unsigned long */
	"%",			/* unsigned long */
	"*=",			/* unsigned long */
	"/=",			/* unsigned long */
	"%=",			/* unsigned long */
	"u_long->double", 	/* unsigned long */
	">>",			/* unsigned long */
	">>=",			/* unsigned long */
	"-",			/* 130 - */};

/*
 * Strings for instruction tables.
 */
char	mov[]	= "mov";
char	clr[]	= "clr";
char	cmp[]	= "c";
char	tst[]	= "tst";
char	add[]	= "a";
char	sub[]	= "s";
char	inc[]	= "inc";
char	dec[]	= "dec";
char	mpy[]	= "mpy";
char	divu[]	= "div";
char	sra[]	= "sra";
char	srl[]	= "srl";
char	sla[]	= "sla";
char	szc[]	= "szc";
char	coc[]	= "coc";
char	soc[]	= "soc";
char	xor[]	= "xor";
char	neg[]	= "neg";
char	inv[]	= "inv";

char	li[]	= "li";
char	ai[]	= "ai";
char	andi[]	= "andi";
char	ori[]	= "ori";

char	lmul[]	= "@lmul";
char	sldiv[]	= "@ldiv";
char	slrem[]	= "@lrem";
char	uldiv[] = "@uldiv";
char	ulrem[] = "@ulrem";
char	ualdiv[] = "@auldiv";
char	ualrem[] = "@aulrem";
char	ultof[] = "@ultof";
char	lsla[]	= "@lsla";
char	lsra[]	= "@lsra";
char	lsrl[]	= "@lsrl";
char	almul[]	= "@almul";
char	aldiv[]	= "@aldiv";
char	alrem[]	= "@alrem";
char	idiv[]	= "@idiv";
char	irem[]	= "@irem";

char	jeq[]	= "bjeq";
char	jne[]	= "bjne";
char	jls[]	= "bjls";
char	jgt[]	= "bjgt";
char	jlt[]	= "bjlt";
char	jgs[]	= "bjgs";
char	jle[]	= "bjle";
char	jhi[]	= "bjh";
char	jlo[]	= "bjl";
char	jhe[]	= "bjhe";
char	jmp[]	= "bjmp";


/*
 * Instruction tables, accessed by
 * I (first operand) or I' (second) macros.
 */

struct instab instab[] = {
{	LOAD,	mov,	tst },
{	ASSIGN,	mov,	clr },
{	EQUAL,	cmp,	tst },
{	NEQUAL,	cmp,	tst },
{	LESSEQ,	cmp,	tst },
{	LESS,	cmp,	tst },
{	GREATEQ,cmp,	tst },
{	GREAT,	cmp,	tst },
{	LESSEQP,cmp,	tst },
{	LESSP,	cmp,	tst },
{	GREATQP,cmp,	tst },
{	GREATP,	cmp,	tst },
{	INCOPS,	inc,	inc },
{	DECOPS,	dec,	dec },
{	PLUS,	add,	inc },
{	ASPLUS,	add,	inc },
{	MINUS,	sub,	dec },
{	ASMINUS,sub,	dec },
{	INCBEF,	add,	inc },
{	DECBEF,	sub,	dec },
{	INCAFT,	add,	inc },
{	DECAFT,	sub,	dec },
{	TIMES,	mpy,	mpy },
{	ASTIMES,mpy,	mpy },
{	DIVIDE,	idiv,	idiv },
{	ASDIV,	idiv,	idiv },
{	MOD,	irem,	irem },
{	ASMOD,	irem,	irem },
{	PTOI,	divu,	divu },
{	RSHIFT,	sra,	lsra },
{	ASRSH,	sra,	lsra },
{	URSH,	srl,	lsrl },
{	ASURSH,	srl,	lsrl },
{	LSHIFT,	sla,	lsla },
{	ASLSH,	sla,	lsla },
{	AND,	szc,	szc },
{	ANDN,	szc,	szc },
{	ASANDN,	szc,	szc },
{	TAND,	szc,	szc },
{	OR,	soc,	soc },
{	ASOR,	soc,	soc },
{	EXOR,	xor,	xor },
{	ASXOR,	xor,	xor },
{	NEG,	neg,	neg },
{	COMPL,	inv,	inv },
{	CALL1,	"",	"" },
{	CALL2,	"",	"" },
{	LTIMES,	lmul,	lmul },
{	LDIV,	sldiv,	sldiv },
{	LMOD,	slrem,	slrem },
{	LASTIMES,almul,	almul },
{	LASDIV,	aldiv,	aldiv },
{	LASMOD,	alrem,	alrem },
{	UDIV,	divu,	divu },
{	UMOD,	divu,	divu },
{	ASUDIV,	divu,	divu },
{	ASUMOD,	divu,	divu },
{	ULTIMES,lmul,	lmul },		/* symmetry */
{	ULDIV,	uldiv,	uldiv },
{	ULMOD,	ulrem,	ulrem },
{	ULASTIMES,almul,almul },	/* symmetry */
{	ULASDIV,ualdiv,	ualdiv },
{	ULASMOD,ualrem,	ualrem },
{	ULTOF,	ultof,	ultof },
{	0,	0,	0 } };

/* Similar table for immediate instructions,
 * accessed as I"
 */
struct instab imm_tab[] = {
{	PLUS,	ai,	"" },
{	ASPLUS,	ai,	"" },
{	MINUS,	ai,	"" },
{	ASMINUS,ai,	"" },
{	INCBEF,	ai,	"" },
{	DECBEF,	ai,	"" },
{	INCAFT,	ai,	"" },
{	DECAFT,	ai,	"" },
{	AND,	andi,	"" },
{	ANDN,	andi,	"" },
{	ASANDN,	andi,	"" },
{	TAND,	andi,	"" },
{	OR,	ori,	"" },
{	ASOR,	ori,	"" },
{	0,	0,	0  }
};

/*
 * Similar table for relationals.
 * The first string is for the positive
 * test, the second for the inverted one.
 */
struct instab branchtab[] = {
{	EQUAL,	jeq,	jne },
{	NEQUAL,	jne,	jeq },

{	LESSEQ,	jls,	jgt },
{	LESS,	jlt,	jgs },
{	GREATEQ,jgs,	jlt },
{	GREAT,	jgt,	jls },

{	LESSEQP,jle,	jhi },
{	LESSP,	jlo,	jhe },
{	GREATQP,jhe,	jlo },
{	GREATP,	jhi,	jle },

{	0,	0,	0 } };
