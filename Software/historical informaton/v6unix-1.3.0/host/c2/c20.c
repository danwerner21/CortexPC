/*
 * C object code improver

 */
#include "c2.h"

static char *memory_base = NULL;
static int memory_len = 256*1024;
static int memory_use = 0;
static int memory_max = 0;

void freeall();

struct optab optab[] = {
  { "bjmp",	BJMP	},
  { "bjeq",	CBR | JEQ<<8 },
  { "bjne",	CBR | JNE<<8 },
  { "bjlt",	CBR | JLT<<8 },
  { "bjls",	CBR | JLS<<8 },
  { "bjgt",	CBR | JGT<<8 },
  { "bjgs",	CBR | JGS<<8 },
  { "bjl",	CBR | JL <<8 },
  { "bjh",	CBR | JH <<8 },
  { "bjle",	CBR | JLE<<8 },
  { "bjhe",	CBR | JHE<<8 },
  { "jnc",	CBR | JNC<<8 },
  { "b",	B	},
  { "bl", 	BL	},
  { "blwp",	BLWP	},
  { "a",	A	},
  { "ab",	A | BYTE<<8 },
  { "abs",	ABS	},
  { "ai",	AI	},
  { "andi",	ANDI	},
  { "c",	C	},
  { "cb",	C | BYTE<<8 },
  { "ci",	CI	},
  { "ckof",	CKOF	},
  { "ckon",	CKON	},
  { "clr",	CLR	},
  { "coc",	COC	},
  { "czc",	CZC	},
  { "dec",	DEC	},
  { "dect",	DECT	},
  { "div",	DIV	},
  { "divs",	DIVS	},
  { "idle",	IDLE	},
  { "inc",	INC	},
  { "inct",	INCT	},
  { "inv",	INV	},
  { "ldcr",	LDCR	},
  { "li",	LI	},
  { "limi",	LIMI	},
  { "lrex",	LREX	},
  { "lst",	LST	},
  { "lwp",	LWP	},
  { "lwpi",	LWPI	},
  { "mov",	MOV	},
  { "movb",	MOV | BYTE<<8 },
  { "mpy",	MPY	},
  { "mpys",	MPYS	},
  { "neg",	NEG	},
  { "ori",	ORI	},
  { "rset",	RSET	},
  { "rtwp",	RTWP	},
  { "s",	S	},
  { "sb",	S | BYTE<<8 },
  { "sbo",	SBO	},
  { "sbz",	SBZ	},
  { "tb",	TB	},
  { "soc",	SOC	},
  { "socb",	SOC | BYTE<<8 },
  { "szc",	SZC	},
  { "szcb",	SZC | BYTE<<8 },
  { "seto",	SETO	},
  { "sla",	SLA	},
  { "sra",	SRA	},
  { "src",	SRC	},
  { "srl",	SRL	},
  { "stcr",	STCR	},
  { "stst",	STST 	},
  { "stwp",	STWP	},
  { "swpb",	SWPB	},
  { "sys",	SYS	},
  { "x",	X	},
  { "xop",	XOP	},
  { "xor",	XOR	},
  { ".globl",	EROU	},
  { ".text",	TEXT	},
  { ".data",	DATA	},
  { ".bss",	BSS	},
  { ".even",	EVEN	},
  { ".end",	END	},
  { 0,		0	},
};

char revbr[] = {JNE, JEQ, JGT, JLT, JGS, JLS, JHE, JLE, JH, JL, JOC, JNC};
int  isn  = 20000;
int  lastseg  = -1;

#define  NSTK  5000

void  opsetup();
int   input();
void  refcount();
void  iterate();
void  comjump();
void  output();
int   get_line();
int   getnum();
int   oplook();
void  reducelit();
void  xjump();
void  backjmp();

int main(argc, argv)
int argc;
char **argv;
{
	register int niter, maxiter, isend;
	int nflag;
	char stspace[NSTK];

	if (argc>1 && argv[1][0]=='+') {
		argc--;
		argv++;
		debug++;
	}
	nflag = 0;
	if (argc>1 && argv[1][0]=='-' && argv[1][1]=='n') {
		argc--;
		argv++;
		nflag++;
	}

	if (argc>1) {
		if (freopen(argv[1], "r", stdin) == NULL) {
			fprintf(stderr, "C2: can't find %s\n", argv[1]);
			exit(1);
		}
	}
	if (argc>2) {
		if (freopen(argv[2], "w", stdout) == NULL) {
			fprintf(stderr, "C2: can't create %s\n", argv[2]);
			exit(1);
		}
	}

	maxiter = 0;
	opsetup();
	/* while there are functions (globals) left... */
	do {
		freeall();
		/* read a function (global) */
		isend = input();
		/* move all segment (.data/.text/.bss) items together */
		movedat();
		niter = 0;
		do {
			/* count label usage & set jump references */
			refcount();
			do {
				/* optimize jumps */
				iterate();
				niter++;
			} while (nchange);
			/* remove common sequences before jumps */
			comjump();
			/* optimize register allocation */
			clearreg();
			rmove(); /* */
		} while (nchange || jumpsw());
		/* print optimised tree to out file */
		output();
		if (niter > maxiter)
			maxiter = niter;
		/* release tree */
		freeall();
	} while (isend);

	if (nflag) {
		fprintf(stderr, "%d iterations\t\t\t", maxiter);
		fprintf(stderr, "%d jumps to jumps\n", nbrbr);
		fprintf(stderr, "%d inst. after jumps\t\t", iaftbr);
		fprintf(stderr, "%d jumps to .+2\n", njp1);
		fprintf(stderr, "%d redundant labels\t\t", nrlab);
		fprintf(stderr, "%d cross-jumps\n", nxjump);
		fprintf(stderr, "%d code motions\t\t\t", ncmot);
		fprintf(stderr, "%d branches reversed\n", nrevbr);
		fprintf(stderr, "%d redundant moves\t\t", redunm);
		fprintf(stderr, "%d simplified addresses\n", nsaddr);
		fprintf(stderr, "%d loops inverted\t\t", loopiv);
		fprintf(stderr, "%d redundant jumps\n", nredunj);
		fprintf(stderr, "%d common seqs before jmp's\t", ncomj);
		fprintf(stderr, "%d skips over jumps\n", nskip);
/*		fprintf(stderr, "%d sob's added\t\t\t", nsob);
		fprintf(stderr, "%d redundant tst's\n", nrtst); */
		fprintf(stderr, "%d literals eliminated\t\t", nlit);
		fprintf(stderr, "%ldK core\n", (((long)memory_max+01777)>>10)&077);
	}
	exit(0);
}

/* Read the source file, function by function (global by global, actually) */
int input()
{
	register struct node *p, *lastp;
	register int oper;

	lastp = &first;
	for (;;) {
		oper = get_line();
		switch (oper&0377) {

		/* LABEL node: either text in 'code' or number in 'labno' */
		case LABEL:
			p = (struct node *)alloc(sizeof first);
			if (line[0] != 'L') {
				p->op = DLABEL;
				p->subop = 0;
				p->labno = 0;
				p->code = copy(1, line);
			}
			else {
				p->op = LABEL;
				p->subop = 0;
				p->labno = getnum(line+1);
				p->code = 0;
			}
			break;

		case BJMP:
		case CBR:
		case SWB:
		case B:
			/* Jumps, too, have their destination as text or number */
			p = (struct node *)alloc(sizeof first);
			p->op = oper&0377;
			p->subop = oper>>8;
			if (*curlp=='L' && (p->labno = getnum(curlp+1)))
				p->code = 0;
			else {
				p->labno = 0;
				p->code = copy(1, curlp);
			}
			break;

		/* all other ops have their operands held as text */
		default:
			p = (struct node *)alloc(sizeof first);
			p->op = oper&0377;
			p->subop = oper>>8;
			p->labno = 0;
			p->code = copy(1, curlp);
			break;

		}
		p->forw = 0;
		p->back = lastp;
		lastp->forw = p;
		lastp = p;
		p->ref = 0;
		/* EROU = .globl, start of a new function */
		if (oper==EROU)
			return(1);
		/* END = end, end of this file */
		if (oper==END)
			return(0);
	}
}

/* Read a 'line' of source into global buffer line. Labels and opcodes are
 * returned separately */
int get_line()
{
	register char *lp;
	register int c;

	lp = line;
	while ((c = getchar())==' ' || c=='\t')
		;
	do {
		if (c==':') {
			*lp++ = 0;
			return(LABEL);
		}
		if (c=='\n') {
			*lp++ = 0;
			return(oplook());
		}
		if (lp >= &line[LSIZE-2]) {
			fprintf(stderr, "C2: Sorry, input line too long\n");
			exit(1);
		}
		*lp++ = c;
	} while ((c = getchar()) != EOF);
	*lp++ = 0;
	return(END);
}

/* do atoi for ap, but return 0 if not a number */
int getnum(ap)
char *ap;
{
	register char *p;
	register int n, c;

	p = ap;
	n = 0;
	while ((c = *p++) >= '0' && c <= '9')
		n = n*10 + c - '0';
	if (*--p != 0)
		return(0);
	return(n);
}

/* Follow the optimised chain of nodes and print out assembly for the
 * function.
 */
void output()
{
	register struct node *t;
	register struct optab *oper;
	register int byte;

	t = &first;
	while ((t = t->forw) != 0) switch (t->op) {

	case END:
		return;

	case LABEL:
		printf("L%d:\n", t->labno);
		continue;

	case DLABEL:
		printf("%s:", t->code);
		if (t->code[0]!='1') putchar('\n');
		continue;

	case TEXT:
	case DATA:
	case BSS:
		lastseg = t->op;

	default:
		if ((byte = t->subop) == BYTE)
			t->subop = 0;
		for (oper = optab; oper->opstring!=0; oper++)
			if ((oper->opcode&0377) == t->op
			 && (oper->opcode>>8) == t->subop) {
				printf("\t%s", oper->opstring);
				if (byte==BYTE)
					printf("b");
				break;
			}
		if (t->code) {
			printf("\t%s\n", t->code);
		} else if (t->op==BJMP || t->op==CBR )
			printf("\tL%d\n", t->labno);
		else
			printf("\n");
		continue;

	case SWB:
		printf("\tL%d\n", t->labno);
		continue;

	case 0:
		if (t->code)
			printf("\t%s\n", t->code);
		continue;
	}
}

/* Copy a string to a new buffer, if na>1 append a second string */
char *
copy(na, ap, ap2)
	int na;
	char *ap, *ap2;
{
	register char *p, *np;
	char *onp;
	register int n;

	p = ap;
	n = 0;
	if (*p==0)
		return(0);
	do
		n++;
	while (*p++);
	if (na>1) {
		p = ap2;
		while (*p++)
			n++;
	}
	onp = np = alloc(n);
	p = ap;
	while ((*np++ = *p++) != 0)
		;
	if (na>1) {
		p = ap2;
		np--;
		while ((*np++ = *p++) != 0);
	}
	return(onp);
}

/* Bernstein hash */
int hash(str)
	char *str;
{
	unsigned int hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;
	return (hash&077777);
}

/* build the hash table for opcodes */
void opsetup()
{
	register struct optab *optp, **ophp;
	register char *p;

	for (optp = optab; (p = optp->opstring) != 0; optp++) {
		ophp = &ophash[hash(p) % OPHS];
		while (*ophp++)
			if (ophp > &ophash[OPHS])
				ophp = ophash;
		*--ophp = optp;
	}
}

/* Look up an opcode and return its opcode */
int oplook()
{
	register struct optab *optp;
	register char *lp, *np;
	static char tmpop[32];
	struct optab **ophp;

	if (line[0]=='\0') {
		curlp = line;
		return(0);
	}
	np = tmpop;
	for (lp = line; *lp && *lp!=' ' && *lp!='\t';)
		*np++ = *lp++;
	*np++ = 0;
	while (*lp=='\t' || *lp==' ')
		lp++;
	curlp = lp;
	ophp = &ophash[hash(tmpop) % OPHS];
	while ((optp = *ophp) != 0) {
		np = optp->opstring;
		lp = tmpop;
		while (*lp == *np++)
			if (*lp++ == 0)
				return(optp->opcode);
		ophp++;
		if (ophp >= &ophash[OPHS])
			ophp = ophash;
	}
	/* Turn labels in data tables to the SWB pseudo instruction */
	if (line[0]=='L') {
		lp = &line[1];
		while (*lp)
			if (*lp<'0' || *lp++>'9')
				return(0);
		curlp = line;
		return(SWB);
	}
	curlp = line;
	return(0);
}

struct node *
nonlab(p)
register struct node *p;
{
	CHECK(10);
	while (p && p->op==LABEL)
		p = p->forw;
	return(p);
}

/* Move all related segments together */
void movedat()
{
	register struct node *p1, *p2;
	struct node *p3;
	register int seg;
	struct node data;
	struct node *datp;

	if (first.forw == 0)
		return;
	if (lastseg != TEXT && lastseg != -1) {
		p1 = (struct node *)alloc(sizeof(first));
		p1->op = lastseg;
		p1->subop = 0;
		p1->code = NULL;
		p1->forw = first.forw;
		p1->back = &first;
		first.forw->back = p1;
		first.forw = p1;
	}
	datp = &data;
	for (p1 = first.forw; p1!=0; p1 = p1->forw) {
		if (p1->op == DATA) {
			p2 = p1->forw;
			while (p2 && p2->op!=TEXT)
				p2 = p2->forw;
			if (p2==0)
				break;
			p3 = p1->back;
			p1->back->forw = p2->forw;
			p2->forw->back = p3;
			p2->forw = 0;
			datp->forw = p1;
			p1->back = datp;
			p1 = p3;
			datp = p2;
		}
	}
	if (data.forw) {
		datp->forw = first.forw;
		first.forw->back = datp;
		data.forw->back = &first;
		first.forw = data.forw;
	}
	seg = lastseg;
	for (p1 = first.forw; p1!=0; p1 = p1->forw) {
		if (p1->op==TEXT||p1->op==DATA||p1->op==BSS) {
			if ((p2 = p1->forw) != 0) {
				if (p2->op==TEXT||p2->op==DATA||p2->op==BSS)
					p1->op  = p2->op;
			}
			if (p1->op == seg || (p1->forw && p1->forw->op==seg)) {
				p1->back->forw = p1->forw;
				p1->forw->back = p1->back;
				p1 = p1->back;
				continue;
			}
			seg = p1->op;
		}
	}
}

void refcount()
{
	register struct node *p, *lp;
	static struct node *labhash[LABHS];
	register struct node **hp, *tp;

	/* hash all numeric labels */
	for (hp = labhash; hp < &labhash[LABHS];)
		*hp++ = 0;
	for (p = first.forw; p!=0; p = p->forw)
		if (p->op==LABEL) {
			labhash[p->labno % LABHS] = p;
			p->refc = 0;
		}
	/* scan the list for usage */
	for (p = first.forw; p!=0; p = p->forw) {
		if (p->op==BJMP || p->op==CBR || p->op==SWB) {
			p->ref = 0;
			lp = labhash[p->labno % LABHS];
			if (lp==0 || p->labno!=lp->labno)
			for (lp = first.forw; lp!=0; lp = lp->forw) {
				if (lp->op==LABEL && p->labno==lp->labno)
					break;
			}
			if (lp) {
				/* if multiple labels for the same destination, use the last one */
				tp = nonlab(lp)->back;
				if (tp!=lp) {
					p->labno = tp->labno;
					lp = tp;
				}
				/* link the jump to the destination, increase label refcount */
				p->ref = lp;
				lp->refc++;
			}
		}
	}
	for (p = first.forw; p!=0; p = p->forw)
		if (p->op==LABEL && p->refc==0 && (lp = nonlab(p))->op && lp->op!=SWB)
			decref(p);
}

void iterate()
{
	register struct node *p, *rp, *p1;

	nchange = 0;
	for (p = first.forw; p!=0; p = p->forw) {
		CHECK(0);
		/* remove jumps to jumps */
		if ((p->op==BJMP||p->op==CBR||p->op==SWB) && p->ref) {
			rp = nonlab(p->ref);
			if (rp->op==BJMP && rp->labno && p->labno!=rp->labno) {
				nbrbr++;
				p->labno = rp->labno;
				decref(p->ref);
				rp->ref->refc++;
				p->ref = rp->ref;
				CHECK(1);
				nchange++;
			}
		}
		/* remove skips over jumps, i.e. reverse condition */
		if (p->op==CBR && (p1 = p->forw)->op==BJMP) {
			rp = p->ref;
			do
				rp = rp->back;
			while (rp->op==LABEL);
			if (rp==p1) {
				decref(p->ref);
				p->ref = p1->ref;
				p->labno = p1->labno;
				p1->forw->back = p;
				p->forw = p1->forw;
				p->subop = revbr[(int) p->subop];
				nchange++;
				CHECK(2);
				nskip++;
			}
		}
		/* remove dead instrucions after an uncond. jump */
		if (p->op==BJMP || p->op==B) {
			while (p->forw && p->forw->op!=LABEL
				&& p->forw->op!=DLABEL
				&& p->forw->op!=EROU && p->forw->op!=END
				&& p->forw->op!=0 && p->forw->op!=DATA
				&& p->forw->op!=BSS) {
				nchange++;
				iaftbr++;
				if (p->forw->ref)
					decref(p->forw->ref);
				p->forw = p->forw->forw;
				p->forw->back = p;
				CHECK(3);
			}
			rp = p->forw;
			/* remove jumps to next instruction */
			while (rp && rp->op==LABEL) {
				if (p->ref == rp) {
					p->back->forw = p->forw;
					p->forw->back = p->back;
					p = p->back;
					decref(rp);
					nchange++;
					CHECK(4);
					njp1++;
					break;
				}
				rp = rp->forw;
			}
		}
		if (p->op==BJMP || p->op==B) {
			xjump(p);
			p = codemove(p);
		}
	}
}

void xjump(p1)
register struct node *p1;
{
	register struct node *p2, *p3;

	if ((p2 = p1->ref)==0)
		return;
	for (;;) {
		while ((p1 = p1->back) && p1->op==LABEL);
		while ((p2 = p2->back) && p2->op==LABEL);
		if (!equop(p1, p2) || p1==p2)
			return;
		p3 = insertl(p2);
		p1->op = BJMP;
		p1->subop = 0;
		p1->ref = p3;
		p1->labno = p3->labno;
		p1->code = 0;
		nxjump++;
		CHECK(5);
		nchange++;
	}
}

struct node *
insertl(oldp)
register struct node *oldp;
{
	register struct node *lp;

	if (oldp->op == LABEL) {
		oldp->refc++;
		return(oldp);
	}
	if (oldp->back->op == LABEL) {
		oldp = oldp->back;
		oldp->refc++;
		return(oldp);
	}
	lp = (struct node *)alloc(sizeof first);
	lp->op = LABEL;
	lp->subop = 0;
	lp->labno = isn++;
	lp->ref = 0;
	lp->code = 0;
	lp->refc = 1;
	lp->back = oldp->back;
	lp->forw = oldp;
	oldp->back->forw = lp;
	oldp->back = lp;
	CHECK(6);
	return(lp);
}

int equop(ap1, p2)
struct node *ap1, *p2;
{
	register char *cp1, *cp2;
	register struct node *p1;

	p1 = ap1;
	if (p1->op!=p2->op || p1->subop!=p2->subop)
		return(0);
	if (p1->op>0 && p1->op<A)
		return(0);
	cp1 = p1->code;
	cp2 = p2->code;
	if (cp1==0 && cp2==0)
		return(1);
	if (cp1==0 || cp2==0)
		return(0);
	while (*cp1 == *cp2++)
		if (*cp1++ == 0)
			return(1);
	return(0);
}

struct node *
codemove(p)
struct node *p;
{
	register struct node *p1, *p2, *p3;
	struct node *t, *tl;
	int n;

	p1 = p;
	if (p1->op!=BJMP || (p2 = p1->ref)==0)
		return(p1);
	while (p2->op == LABEL)
		if ((p2 = p2->back) == 0)
			return(p1);
	if (p2->op!=BJMP && p2->op!=B)
		goto ivloop;
	if (p1==p2)
		return(p1);
	p2 = p2->forw;
	p3 = p1->ref;
	while (p3) {
		if (p3->op==BJMP || p3->op==B) {
			if (p1==p3 || p1->forw==p3 || p1->back==p3)
				return(p1);
			ncmot++;
			nchange++;
			CHECK(70);
			p1->back->forw = p2;
			p1->forw->back = p3;
			p2->back->forw = p3->forw;
			p3->forw->back = p2->back;
			p2->back = p1->back;
			p3->forw = p1->forw;
			decref(p1->ref);
			CHECK(7);
			return(p2);
		} else
			p3 = p3->forw;
	}
	return(p1);
ivloop:
	if (p1->forw->op!=LABEL)
		return(p1);
	p3 = p2 = p2->forw;
	n = 16;
	do {
		if ((p3 = p3->forw) == 0 || p3==p1 || --n==0)
			return(p1);
	} while (p3->op!=CBR || p3->labno!=p1->forw->labno);
	do
		if ((p1 = p1->back) == 0)
			return(p);
	while (p1!=p3);
	p1 = p;
	tl = insertl(p1);
	p3->subop = revbr [(int) p3->subop];
	decref(p3->ref);
	p2->back->forw = p1;
	p3->forw->back = p1;
	p1->back->forw = p2;
	p1->forw->back = p3;
	t = p1->back;
	p1->back = p2->back;
	p2->back = t;
	t = p1->forw;
	p1->forw = p3->forw;
	p3->forw = t;
	p2 = insertl(p1->forw);
	p3->labno = p2->labno;
	p3->ref = p2;
	decref(tl);
	if (tl->refc<=0)
		nrlab--;
	loopiv++;
	nchange++;
	CHECK(8);
	return(p3);
}

/* remove common sequences before jumps, by altering the second jump */
void comjump()
{
	register struct node *p1, *p2, *p3;

	for (p1 = first.forw; p1!=0; p1 = p1->forw)
		if (p1->op==BJMP && (p2 = p1->ref) && p2->refc > 1)
			for (p3 = p1->forw; p3!=0; p3 = p3->forw)
				if (p3->op==BJMP && p3->ref == p2)
					backjmp(p1, p3);
}

void backjmp(ap1, ap2)
struct node *ap1, *ap2;
{
	register struct node *p1, *p2, *p3;

	p1 = ap1;
	p2 = ap2;
	for(;;) {
		while ((p1 = p1->back) && p1->op==LABEL);
		p2 = p2->back;
		if (equop(p1, p2)) {
			p3 = insertl(p1);
			p2->back->forw = p2->forw;
			p2->forw->back = p2->back;
			p2 = p2->forw;
			decref(p2->ref);
			p2->labno = p3->labno;
			p2->ref = p3;
			nchange++;
			ncomj++;
			CHECK(9);
		} else
			return;
	}
}

int abs(x)
register int x;
{
	return(x<0? -x: x);
}

int jumpsw()
{
	register struct node *p, *p1;
	register int t;
	register struct node *tp;
	int nj;

	t = 0;
	nj = 0;
	for (p=first.forw; p!=0; p = p->forw)
		p->refc = ++t;
	for (p=first.forw; p!=0; p = p1) {
		p1 = p->forw;
		if (p->op == CBR && p1->op==BJMP && p->ref && p1->ref
		 && abs(p->refc - p->ref->refc) > abs(p1->refc - p1->ref->refc)) {
			if (p->ref==p1->ref)
				continue;
			p->subop = revbr [(int) p->subop];
			tp = p1->ref;
			p1->ref = p->ref;
			p->ref = tp;
			t = p1->labno;
			p1->labno = p->labno;
			p->labno = t;
			nrevbr++;
			nj++;
		}
	}
	return(nj);
}

#define round(a,b) ((((a)+(b)-1)/(b))*(b))

void freeall() {
	memory_use = 0;
}

char *
alloc(nn)
int nn;
{
	register char *p;
	register int n = nn;

	n=round(n,sizeof(char *));
	if (memory_base == NULL) {
		memory_base = (char *)malloc(memory_len);
		if (memory_base == NULL) {
			fprintf(stderr, "C Optimizer: out of memory\n");
			exit(1);
		}
	}
	if (memory_use + n > memory_len) {
		fprintf(stderr, "C Optimizer: out of space %d+%d:%d\n",memory_use,n,memory_len);
		exit(1);
	}
	p = memory_base + memory_use;
	memory_use += n;
	if (memory_use > memory_max) {
		memory_max = memory_use;
	}
	return(p);
}

void decref(p)
register struct node *p;
{
	if (--p->refc <= 0) {
		nrlab++;
		p->back->forw = p->forw;
		p->forw->back = p->back;
	}
}
