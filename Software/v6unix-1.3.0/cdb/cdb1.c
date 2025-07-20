/*

	C debugger

*/

#include <stdio.h>
#include "param.h"
#include "user.h"

#define	DSP	0
#define	ISP	1
#define	NBKP	10
#define	SYMSIZ	12*1000
#define	BADJST	01

int	fcore;
int	errno;
int	fsym;
unsigned int	symoff;
char	*lp;
int	errflg;
int	symlen;
int	symct;
int	symcor;
int	symbuf[SYMSIZ];
int	*symptr;
struct {
	int	loc;
	int	ins;
	int	count;
	int	flag;
} bkptl[NBKP];
int	lastbp;
char	symbol[8];
int	symflg;
int	symval;
char	tsym[8];
char	fsymbol[10];
char	ssymbol[8];
int	ssymflg;
int	ssymval;
int	signo;
char	line[128];
int	regbuf[512];
int	*uregs	= &regbuf[512];
char	*rtsize;
int	loccsv;
int	base;

#define	RUSER	1
#define	RIUSER	2
#define	WUSER	4
#define	RUREGS	3
#define	WUREGS	6
#define	SETTRC	0
#define	CONTIN	7
#define	EXIT	8

#define DLYTRP	0x200

#define	R0	(-7)
#define	R1	(-6)
#define	R2	(-5)
#define	R3	(-4)
#define	R4	(-17)
#define	R5	(-18)
#define	R6	(-19)
#define	R7	(-20)
#define	R8	(-21)
#define	R9	(-22)
#define R10	(-23)
#define	R11	(-3)
#define	R12	(-2)
#define	R13	(-1)
#define	BP	(-14)
#define XEA     (-12)
#define	WP	(-11)
#define	SP	(-10)
#define	PC	(-9)
#define ST	(-8)

struct reglist {
	char	*rname;
	int	roffs;
} reglist[] = {
	"xea", XEA,
	"ps",  ST,	"pc",  PC,
	"sp",  SP,	"bp",  BP,
	"r13", R13,	"r12", R12,
	"r11", R11,	"r10", R10,
	"r9",  R9,	"r8",  R8,
	"r7",  R7,	"r6",  R6,
	"r5",  R5,	"r4",  R4,
	"r3",  R3,	"r2",  R2,
	"r1",  R1,	"r0",  R0
};

#ifndef __ti990__
struct sfregs {
	int	junk[2];
	int	fpsr;
	float	sfr[6];
};

struct lfregs {
	int	junk[2];
	int	fpsr;
	double	lfr[6];
};
#endif

int	frnames[] = { 0, 3, 4, 5, 1, 2 };
int	dot;
int	tdot;
int	dotinc = 2;
int	lastcom = '/';
int	lastype	= 'x';
int	modifier;
char	*symfil	= "a.out";
char	*corfil	= "core";
int	callist[50];
int	entpt[50];
int	callev;
int	pid;
int	adrflg;
int	idsep;
char	*signals[] = {
	"",
	"Hangup",
	"Interrupt",
	"Quit",
	"Illegal instruction",
	"Trace/BPT",
	"IOT",
	"EMT",
	"Floating exception",
	"Killed",
	"Bus error",
	"Memory fault",
	"Bad system call",
	"",
	"",
	"",
};

main(argc, argv)
char **argv;
{
	int onintr();

	if (argc>1)
		symfil = argv[1];
	if (argc>2)
		corfil = argv[2];
	fcore = open(corfil, 0);
	if ((fsym = open(symfil, 0)) < 0) {
		printf("%s not found\n", symfil);
		return;
	}
	read(fsym, regbuf, 020);
	if (regbuf[0]==0411)			/* I/D separated */
		idsep++;
	else if (regbuf[0]!=0410 && regbuf[0]!=0407) {	/* magic */
		printf("Bad format: %s\n", symfil);
		return;
	}
	symoff = regbuf[1] + regbuf[2];
	symlen = regbuf[4];
	if (regbuf[7] != 1)
		symoff <<= 1;
	symoff += 020;
	lseek(fsym, (long)symoff, 0);
	symcor = read(fsym, symbuf, sizeof symbuf);
	if (symcor>0)
		symoff += symcor;
	symcor >>= 1;
	base = regbuf[5];
	read(fcore, regbuf, 1024);
	signo = regbuf[0].u_arg[0]&017;
/*	regbuf->u_tsize <<= 6;
	regbuf->u_dsize <<= 6;
	regbuf->u_ssize <<= 6; */
regbuf->u_tsize = regbuf[1];
regbuf->u_dsize = regbuf[2];
regbuf->u_ssize = regbuf[3];
	
	rtsize = (regbuf->u_tsize+017777) & ~017777;
	if (symlook("csv\0\0\0\0"))
		loccsv = ssymval;
	setstack();
	signal(SIGINS, onintr);
	setexit();
/*	signal(SIGINT, onintr); */
loop:
	if (errflg) {
		printf("?\n");
		errflg = 0;
	}
	fflush(stdout);
	lp = line;
	while ((*lp = getchar()) != '\n') {
		if (*lp++ < 0) {
			if (pid)
				ptrace(EXIT, pid, 0, 0);
			return;
		}
	}
	lp = line;
	command();
	goto loop;
}

command()
{
	register n;

	adrflg = expr();
	if (errflg)
		return;
	n = getcnt();
	if (lastcom=='$')
		lastcom = '/';
	if (*lp == '\n') {
		if (!adrflg)
			dot += dotinc;
	} else
		lastcom = *lp++;
	modifier = 0;
	if (*lp != '\n')
		modifier = *lp++;
	if (lastcom=='%' && modifier=='r') {
		runcom();
		return;
	}
	if (*lp != '\n') {
		errflg++;
		return;
	}
	if (adrflg)
		dot = tdot;
	while(n) {
		scommand(n);
		if (errflg)
			return;
		if (--n)
			dot += dotinc;
	}
}

scommand(n)
{
	register w, c;
	struct { int i[4]; };
	int onintr();
#ifndef __ti990__
	double fw;
#endif

	switch(lastcom) {

	case '/':
		w = cget(dot, DSP);
		if (modifier)
			lastype = modifier;
		switch(lastype) {

		case 'o':
			printf("%.1o\n", w);
			dotinc = 2;
			return;

		case 'i':
			printf("%d\n", w);
			dotinc = 2;
			return;

		case 'x':
			printf("%x\n", w);
			dotinc = 2;
			return;

#ifndef __ti990__
		case 'f':
			fw = 0;
			fw.i[0] = w;
			fw.i[1] = cget(dot+2, DSP);
			printf("%e\n", fw);
			dotinc = 4;
			return;

		case 'd':
			fw.i[0] = w;
			fw.i[1] = cget(dot+2, DSP);
			fw.i[2] = cget(dot+4, DSP);
			fw.i[3] = cget(dot+6, DSP);
			printf("%e\n", fw);
			dotinc = 8;
			return;
#endif
		}
		errflg++;
		return;

	case '\\':
		printf("%.1o\n", cget(dot, DSP)&0377);
		dotinc = 1;
		return;

	case '=':
		printf("dec: %d,\thex: %x,\toct: %o\n", dot, dot, dot);
		return;

	case '\'':
		printc((cget(dot, DSP)>>8) & 0377);
		if (n<=1)
			putchar('\n');
		dotinc = 1;
		return;

	case '"':
		w = cget(dot, DSP);
		while(c = (cget(w++, DSP)>>8)&0377)
			printc(c);
		putchar('\n');
		return;

	case '&':
		psymoff(cget(dot, DSP), 0100000);
		printf("\n");
		return;

	case '$':
		printf("%s\n", signals[signo]);
		printtrace();
		return;

	case '?':
		printins(0);
		printf("\n");
		return;

	case '%':
		runcom();
		signal(SIGINT, onintr);
		return;

	}
	errflg++;
}

getcnt()
{
	register t1, t2;

	if (*lp != ',')
		return(1);
	lp++;
	t1 = tdot;
	if (expr() == 0) {
		tdot = t1;
		return(1);
	}
	t2 = tdot;
	tdot = t1;
	return(t2);
}

cget(n, space)
{
	register w;

	w = get(n, space);
	if (errflg)
		reset();
	return(w);
}

printc(c)
{
	if (c<' ' || c>'~')
		printf("\\%o", c);
	else
		printf("%c", c);
}

expr()
{
	int i, t1, t2, donef, lastop, b;

	tdot = 0;
	adrflg = 0;
	lastop = '+';
	ssymval = 0;
	donef = 0;
loop:
	fsymbol[0] = 0;
	if (symchar(0)) {
		adrflg++;
		symcollect('_');
		if (*lp++==':' && symchar(0)) {
			for (i=0; i<8; i++)
				fsymbol[i] = tsym[i];
			fsymbol[0] = '~';
			symcollect(0);
		} else 
			lp--;
		if (symlook(tsym) == 0) {
			errflg++;
			reset();
		}
		goto loop;
	}
	if (*lp>='0' && *lp<='9') {
		adrflg++;
		ssymval = 0;
		if (*lp == '0')
			b = 8;
		else
			b = 10;
		while (*lp>='0' && *lp<='9') {
			ssymval *= b;
			ssymval += *lp++ -'0';
		}
		goto loop;
	}
	switch (*lp) {

	default:
		donef++;

	case '+':
	case '-':
		switch(lastop) {

		case '+':
			tdot += ssymval;
			goto op;

		case '-':
			tdot -= ssymval;

		op:
			if (donef)
				return(adrflg);
			else
				lastop = *lp++;
		}
		goto loop;

	case ' ':
	case '\t':
		lp++;
		goto loop;

	case '[':
		lp++;
		t1 = ssymval;
		t2 = tdot;
		if (expr() == 0)
			tdot = 0;
		ssymval = cget(t1 + (tdot<<1), DSP);
		tdot = t2;
		if (*lp == ']')
			lp++;
		goto loop;
	}
}

symcollect(c)
{
	register char *p;

	p = tsym;
	if (c)
		*p++ = c;
	while (symchar(1)) {
		if (p < &tsym[8])
			*p++ = *lp;
		lp++;
	}
	while (p < &tsym[8])
		*p++ = 0;
}

symchar(dig)
{
	if (*lp>='a'&&*lp<='z' || *lp=='_')
		return(1);
	if (dig && *lp>='0' && *lp<='9')
		return(1);
	return(0);
}

error()
{
	errflg++;
	reset();
}

printtrace()
{
	int tpc, tbp, narg, argp, i;

	if (modifier=='r') {
		printregs();
		return;
	}
#ifndef __ti990__
	if (modifier=='f' || modifier=='d') {
		printfregs();
		return;
	}
#endif
	tpc = uregs[PC];
	tbp = uregs[BP];
	callev = 0;
	while (errflg == 0) {
		narg = findroutine(tpc, tbp);
		printf("%2d: %.8s(", callev, ssymbol);
		if (--narg >= 0)
			printf("%.1o", get(tbp+22, DSP));
		argp = tbp+22;
		while(--narg >= 0)
			printf(",%.1o", get(argp += 2, DSP));
		printf(")\n");
		tpc = get(tbp+20, DSP);
		if (callev < 50) {
			entpt[callev] = ssymval;
			callist[callev++] = tbp;
		}
		if ((tbp = get(tbp+18, DSP)) == 0)
			break;
	}
}

setstack()
{
	register int tpc, tbp, i;

	tpc = uregs[PC];
	tbp = uregs[BP];
	callev = 0;
	while (errflg == 0) {
		findroutine(tpc, tbp);
		tpc = get(tbp+2, DSP);
		if (callev >= 50)
			break;
		entpt[callev] = ssymval;
		callist[callev++] = tbp;
		if ((tbp = get(tbp, DSP)) == 0)
			break;
	}
	errflg = 0;
}

#ifndef __ti990__
printfregs()
{
	register i;
	double f;

	printf("fpsr	%.1o\n", regbuf[0].fpsr);
	for (i=0; i<6; i++) {
		if (regbuf[0].fpsr&0200)	/* long mode */
			f = regbuf[0].lfr[frnames[i]];
		else
			f = regbuf[0].sfr[frnames[i]];
		printf("fr%d	%e\n", i, f);
	}
}
#endif

printregs()
{
	register struct reglist *p;
	register char *v, *d;

	for (p=reglist; p < &reglist[19]; p++) {
		printf("%s	%04x", p->rname, v=uregs[p->roffs]);
		d = vallook(v);
		if (d < 010000) {
			printf("	%.8s", ssymbol);
			if (d)
				printf("+%d", d);
		}
		printf("\n");
	}
}

findroutine(rpc, rbp)
{
	register callpt, inst, narg;

	callpt = get(rbp+20, DSP);
	inst=get(callpt-4, ISP);
	if ((inst&~0x3f)==0x0680)	/* bl ... */
		narg = 1;		/* could be 0 ... */
	else {
		errflg++;
		return(0);
	}
	inst = vallook(get(callpt-2, ISP));
	if (inst) {
		ssymbol[0] = '?';
		ssymbol[1] = 0;
		ssymval = 0;
	}
	inst = get(callpt, ISP);
	if (inst == 0x05cf)		/* inct sp */
		return(narg+1);
	if (inst == 0x8fff)		/* c (sp)+,(sp)+ */
		return(narg+2);
	if (inst == 0x022f)		/* ai sp,n */
		return(narg+get(callpt+2, ISP)/2);
	return(narg);
}

runcom()
{
	int stat;
	register w, i;

	switch(modifier) {


	/* delete breakpoint */
	case 'd':
		if (adrflg==0)
			error();
		for (w=0; w<NBKP; w++) {
			i = bkptl[w].loc;
			if (bkptl[w].flag & BADJST)
				i -= 6;
			if (dot==i) {
				if (lastbp==bkptl[w].loc) {
					ptrace(WUREGS,pid,2*(512+ST),uregs[ST]&~020);
					lastbp = 0;
				} else {
					ptrace(WUSER,pid,bkptl[w].loc,bkptl[w].ins);
				}
				bkptl[w].loc = 0;
				bkptl[w].flag = 0;
				return;
			}
		}
		error();

	/* set breakpoint */
	case 'b':
		if (adrflg==0)
			error();
		for (w=0; w<NBKP; w++) {
			i = bkptl[w].loc;
			if (bkptl[w].flag&BADJST)
				i -= 6;
			if (i==dot)
				return;
		}
		for (w=0; w<NBKP; w++)
			if (bkptl[w].loc==0) {
				bkptl[w].loc = dot;
				return;
			}
		error();

	/* run program */
	case 'r':
		lastbp = 0;
		if (pid) {
			ptrace(EXIT, pid, 0, 0);
			pid = 0;
		}
		if ((pid = fork())==0) {
			ptrace(SETTRC, 0, 0, 0);
			/*bground();/* LSX only */
			signal(SIGINT, 0);
			signal(SIGINS, 0);
			doexec();
			printf("Can't execute %s\n", symfil);
			exit(0);
		}
		/*pid = 3; /* On LSX, NPROC = pid of background process */
		bpwait(0);
		setbp(1);
		/*ptrace(WUREGS, pid, 2*(512+ST), 0170000); /* force user mode */

	case 'c':
		if (pid==0)
			error();
		setbp(0);
		if (lastbp) {
			w = lastbp;
			ptrace(CONTIN, pid, 0, 0);
			bpwait(0);
			ptrace(WUSER, pid, w, 0x2c00);
			ptrace(WUREGS, pid, 2*(512+XEA), uregs[XEA]&~DLYTRP);
			lastbp = 0;
		}
		ptrace(CONTIN, pid, 0, 0);
		bpwait(1);
		w = uregs[PC] -= 2;
		for (i=0; i<NBKP; i++)
			if (bkptl[i].loc == w)
				break;
		if (i >= NBKP) {
			printf("%s\n", signals[signo]);
			return;
		}
		lastbp = w;
		ptrace(WUSER, pid, w, bkptl[i].ins);
		ptrace(WUREGS, pid, 2*(512+PC), w);
		ptrace(WUREGS, pid, 2*(512+XEA), uregs[XEA]|DLYTRP);
		printf("Breakpoint: ");
		psymoff(w, 0777);
		printf("\n");
		return;
	}
	error();
}

doexec()
{
	extern _exectrap;
	char *argl[32];
	register char *p, **ap;
	register c;

	_exectrap++;
	ap = argl;
	*ap++ = symfil;
	p = lp;
	do {
		while (*p==' ')
			p++;
		if (*p=='\n' || *p=='\0')
			break;
		*ap++ = p;
		while (*p!=' ' && *p!='\n')
			p++;
		c = *p;
		*p++ = '\0';
	} while (c != '\n');
	*ap++ = 0;
	execv(symfil, argl);
}

setbp(runflag)
{
	register w, i1, l;
	int i2;

	for (w=0; w<NBKP; w++) {
		l = bkptl[w].loc;
		if (l && (runflag||bkptl[w].ins==0)) {
			i1 = ptrace(RUSER, pid, l, 0);
			if (i1==0xc00b) {	/* mov r11,r0 */
				i2 = ptrace(RUSER, pid, l+2, 0);
				if (i2==0x06a0) {	/* bl ... */
					i2 = ptrace(RUSER, pid, l+4, 0);
					if (loccsv == i2) { /* bl @csv */
						l += 6;
						bkptl[w].loc = l;
						bkptl[w].flag |= BADJST;
						i1 = ptrace(RUSER, pid, l, 0);
					}
				}
			}
			bkptl[w].ins = i1;
			ptrace(WUSER, pid, l, 0x2c00); /* xop 0,0 == breakpoint */
			if (errno) {
				printf("Can't set breakpoint ");
				psymoff(bkptl[w].loc);
				printf("\n");
			}
		}
	}
}

bpwait(f)
{
	extern int onintr();
	register w;
	int stat;

    loop:
	signal(SIGINT, 1);
	while ((w = wait(&stat))!=pid && w != -1);
	signal(SIGINT, onintr);
	if (w == -1) {
		ptrace(EXIT, pid, 0, 0);
		pid = 0;
		printf("Wait error\n");
		reset();
	}
	if ((stat & 0377) != 0177) {
		if (signo = stat&0177)
			printf("%s\n", signals[signo]);
		printf("Process terminated.\n");
		if (pid == w) {
			pid = 0;
			reset();
		}
		goto loop;
	}
	signo = stat>>8;
	if (f) collinfo();
	if (signo!=SIGTRC) {
		printf("%s\n", signals[signo]);
		reset();
	}
}

collinfo()
{
	register i;

	/*printf("getting regs ");/* LSX only */
	for (i=0; i<19; i++) {
		uregs[reglist[i].roffs] =
		    ptrace(RUREGS, pid, 2*(512+reglist[i].roffs), 0);
		/*printf(".");
		  fflush(stdout); /* LSX only */
	}
	/*printf("\n");/* LSX only */
	setstack();
}

symlook(symstr)
char *symstr;
{
	register i;
	register symv;

	symset();
	if (fsymbol[0]==0) {
		while(symget()) {
			if (eqstr(symbol, symstr)) {
				savsym();
				return(1);
			}
		}
		return(0);
	}
	while (symget()) {
		/* wait for function symbol */
		if (symbol[0]!='~' || !eqstr(symbol, fsymbol))
			continue;
		symv = symval;
		while (symget()&& symbol[0]!='~' &&symflg!=037)
			if (eqstr(symbol, symstr))
				return(localsym(symv));
		return(0);
	}
}

localsym(s)
{
	register i, xr5;

	/* label, static */
	if (symflg>=2 && symflg<=4) {
		ssymval = symval;
		return(1);
	}
	/* auto, arg */
	if (symflg==1) {
		for (i=0; i<callev; i++)
			if (entpt[i]==s) {
				ssymval = symval+callist[i];
				return(1);
			}
		return(0);
	}
	/* register */
	if (symflg==20) {
		for (i=0; i<callev; i++)
			if (entpt[i]==s) {
				if (i==0) {
					return(0); /* temp, no reg lvalue */
				}
				ssymval = callist[i-1] - 10 + 2*symval;
				return(1);
			}
		return(0);
	}
	return(0);
}

eqstr(as1, as2)
int *as1, *as2;
{
	register char *s1, *s2, *es1;

	s1 = as1;
	s2 = as2;
	for (es1 = s1+8; s1 < es1; )
		if (*s1++ != *s2++)
			return(0);
	return(1);
}

vallook(value)
char *value;
{
	register char *diff;

	diff = 0177777;
	symset();
	while (symget())
		if (symflg&040 && value-symval<=diff) {
			if (symflg==1 && value!=symval)
				continue;
			savsym('_');
			diff = value-symval;
		}
	return(diff);
}

get(aaddr, space)
char *aaddr;
{
	int w;
	register int w1;
	register char *addr;

	addr = aaddr;
	if (pid) {		/* tracing on? */
		w = ptrace(space==DSP?RUSER:RIUSER, pid, addr, 0);
		if (((int)addr)&01) {
			w1 = ptrace(space==DSP?RUSER:RIUSER, pid, addr+1, 0);
			w = (w1>>8)&0377 | (w<<8);
		}
		errflg = errno;
		return(w);
	}
	w = 0;
	if (idsep==0&&(addr-base)<regbuf->u_tsize || idsep&&space==ISP) {
		lseek(fsym, (long)(addr-base)+020, 0);
		if (read(fsym, &w, 2) != 2)
			errflg++;
		return(w);
	}
	if (addr < rtsize+regbuf->u_dsize) {
		if (idsep==0)
			addr -= rtsize;
	} else if (-addr < regbuf->u_ssize)
		addr += regbuf->u_dsize + regbuf->u_ssize;
	else
		errflg++;
	lseek(fcore, (long)addr+1024, 0);
	if (read(fcore, &w, 2) < 2)
		errflg++;
	return(w);
}

symset()
{
	symct = symlen;
	symptr = symbuf;
	lseek(fsym, (long)symoff, 0);
}

symget()
{
	register int *p, *q;
	if ((symct -= 12) < 0)
		return(0);
	if (symptr < &symbuf[symcor]) {
		p = symptr;
		for (q=symbol; q <= &symval;)
			*q++ = *p++;
		symptr = p;
		return(1);
	}
	return(read(fsym, symbol, 12) == 12);
}

savsym(skip)
{
	register int ch;
	register char *p, *q;

	p = symbol;
	q = ssymbol;
	while (p<symbol+8 && (ch = *p++)) {
		if (ch == skip) {
			skip = -1;
			continue;
		}
		*q++ = ch;
	}
	while (q < ssymbol+8)
		*q++ = '\0';
	ssymflg = symflg;
	ssymval = symval;
}

onintr()
{
	putchar('\n');
	errflg++;
	reset();
}

